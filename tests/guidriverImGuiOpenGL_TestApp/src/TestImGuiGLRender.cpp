/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
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

#include "imgui.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGuiOpenGL_TestSupport.h"
#include "TestImGuiGLScenes.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
#include "guidriverImGuiOpenGL_TestEglWindow.h"
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUIDRIVERIMGUI;

namespace {

/*
 *  Tolerance policy of the GPU captures, as the RmlUi render tests of gucefGUI_TestApp use it: what is drawn with
 *  rectangles of the retained widgets (fills, strokes, images, text quads) equals the CSoftwareRenderer reference within
 *  RetainedTolerance per channel and without a different pixel; what Dear ImGui draws itself (rounded corners,
 *  anti-aliased lines, glyph edges) has the same triangles as the reference and differs by the rounding of the
 *  interpolation of the GPU: within WindowTolerance per channel, and WindowMaxDifferent of a thousand pixels
 *  may exceed it.
 */
const Int32 RetainedTolerance = 2;
const Int32 WindowTolerance = 3;
const UInt32 WindowMaxDifferentPerMille = 5;

/** A window of the test with the context of the GL driver made in it */
struct SGlTestWindow
{
    GUI::TWindowContextPtr windowPtr;
    CTestEglWindow* window;
    GUI::TGuiContextPtr contextPtr;
    GUIDRIVERIMGUIGL::CImGuiOpenGLContext* context;

    SGlTestWindow( void ) : windowPtr(), window( GUCEF_NULL ), contextPtr(), context( GUCEF_NULL ) {}

    /** Makes the window and a context in it; false when the machine has no such GL (the test is skipped) */
    bool Make( UInt32 width, UInt32 height, bool gles )
    {
        window = GUCEF_NEW CTestEglWindow( gles );
        windowPtr = GUI::TWindowContextPtr( window );
        if ( !window->Initialize( width, height ) )
            return false;
        return MakeContext();
    }

    bool MakeContext( void )
    {
        window->MakeCurrent();
        contextPtr = GetOpenGLDriver().CreateGUIContext( windowPtr );
        context = static_cast< GUIDRIVERIMGUIGL::CImGuiOpenGLContext* >( AsImGui( contextPtr ) );
        if ( GUCEF_NULL != context )
            context->SetLogicalResolution( 640, 480 );
        return GUCEF_NULL != context;
    }

    void DestroyContext( void )
    {
        if ( GUCEF_NULL != window )
            window->MakeCurrent();
        contextPtr = GUI::TGuiContextPtr();
        context = GUCEF_NULL;
    }

    bool Capture( IMAGE::TPixelMapPtr& pixels )
    {
        window->MakeCurrent();
        return context->CaptureFrame( pixels );
    }

    void Frame( UInt32 count = 1 )
    {
        window->MakeCurrent();
        for ( UInt32 i=0; i<count; ++i )
            context->RenderFrame( 16 );
    }
};

/** The scripted ImGui window and the retained scene in a context, and the same in a software context for the reference */
struct SScriptedScene
{
    GUI::TGuiContextPtr referencePtr;
    CImGuiContext* reference;
    CTestFrameBuilder referenceBuilder;
    CTestFrameBuilder glBuilder;

    SScriptedScene( CTestFrameBuilder::TBuildFunction function )
        : referencePtr(), reference( GUCEF_NULL ), referenceBuilder( function, GUCEF_NULL ), glBuilder( function, GUCEF_NULL )
    {
        referencePtr = CreateSoftwareContext();
        reference = AsImGui( referencePtr );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGLRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Render" );

    bool haveGl = false;
    GUCEF_TESTFW_TESTCASE( "Render: the driver makes a context in an offscreen window of the window driver EGL, which captures what it draws" )
    try
    {
        if ( !LoadFreeTypeFontPlugin() )
            GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );

        // the way an application does it: window backend and GUI driver as plugins
        GUI::CWindowManager& windows = GUI::CGuiGlobal::Instance()->GetWindowManager();
        GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
        if ( windows.GetBackend( "EGL" ).IsNULL() )
            LoadTestPlugin( "guidriverEGL", "guidriver" );
        if ( GUCEF_NULL == manager.GetGuiDriver( "ImGui" ) )
            LoadTestPlugin( "guidriverImGuiOpenGL", "guidriver" );
        GUI::TWindowManagerBackendPtr backend = windows.GetBackend( "EGL" );
        if ( backend.IsNULL() )
            GUCEF_TESTFW_SKIP( "there is no EGL window driver, so no offscreen GL" );
        GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", 640, 480, false );
        if ( window.IsNULL() )
            GUCEF_TESTFW_SKIP( "no OpenGL context can be made on this machine" );

        GUI::TGuiContextPtr contextPtr = manager.CreateGUIContext( "ImGui", window );
        ASSERT_TRUE( !contextPtr.IsNULL() );
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            haveGl = true;
            ASSERT_TRUE( "GUCEF::GUIDRIVERIMGUIGL::CImGuiOpenGLContext" == context->GetClassTypeName() );
            ASSERT_TRUE( "ImGui" == context->GetDriver()->GetDriverName() );
            ASSERT_TRUE( !context->GetDriver()->GetDriverProperty( "Renderer" ).IsNULLOrEmpty() );
            context->SetLogicalResolution( 640, 480 );
            BuildMixedScene( context );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( context->CaptureFrame( pixels ) );
            ASSERT_TRUE( !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() && 480 == pixels->GetHeightInPixels() );
            ASSERT_TRUE( Near( PixelAt( pixels, 5, 5 ), GUI::CColour( 20, 40, 60, 255 ), 1 ) );
            IMAGE::TPixelMapPtr presented;
            ASSERT_TRUE( context->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( !presented.IsNULL() && 640 == presented->GetWidthInPixels() );
            ASSERT_TRUE( Near( PixelAt( presented, 5, 5 ), GUI::CColour( 20, 40, 60, 255 ), 1 ) );
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

    if ( !haveGl )
        return;
    haveGl = false;

    GUCEF_TESTFW_TESTCASE( "Render: the backend runs on a GL context of the test too (OpenGL 3.3 core), and a retained scene of every kind of drawing has the pixels of the software reference" )
    try
    {
        SGlTestWindow gl;
        if ( !gl.Make( 640, 480, false ) )
            GUCEF_TESTFW_SKIP( "no OpenGL 3.3 core context can be made on this machine" );
        haveGl = true;
        ASSERT_TRUE( "#version 330 core" == gl.context->GetGlslVersion() );
        BuildMixedScene( gl.context );
        gl.Frame();
        IMAGE::TPixelMapPtr expected, actual;
        ASSERT_TRUE( gl.Capture( actual ) );
        ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *gl.context, expected ) );
        UInt32 maxDifference = 0;
        const UInt32 perMille = DifferentPerMille( expected, actual, RetainedTolerance, maxDifference );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Mixed scene: " << perMille << " per mille of the pixels above the tolerance, the largest difference " << maxDifference );
        ASSERT_TRUE( 0 == CountDifferences( expected, actual, RetainedTolerance, GUI::CRect(), "mixed scene" ) );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    if ( !haveGl )
        return;

    GUCEF_TESTFW_TESTCASE( "Render: at a render scale of 2 the capture has the render size and pictures and fonts of density 2 have the pixels of the reference" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        BuildDensityScene( gl.context );
        gl.Frame();
        IMAGE::TPixelMapPtr expected, actual;
        ASSERT_TRUE( gl.Capture( actual ) );
        ASSERT_TRUE( !actual.IsNULL() && 1280 == actual->GetWidthInPixels() && 960 == actual->GetHeightInPixels() );
        ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *gl.context, expected ) );
        // the line of the draw surface: the reference walks it in pixels of the render scale, the painter in logical pixels
        ASSERT_TRUE( 0 == CountDifferences( expected, actual, RetainedTolerance, GUI::CRect( 240, 20, 80, 80 ), "density scene" ) );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: a scripted window of Dear ImGui next to the retained scene has the triangles of the software reference, within the tolerance of the GPU" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        SScriptedScene scene( &BuildScriptedWindow );
        ASSERT_TRUE( GUCEF_NULL != scene.reference );
        if ( GUCEF_NULL == scene.reference )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        BuildMixedScene( gl.context );
        BuildMixedScene( scene.reference );
        scene.glBuilder.Attach( gl.contextPtr );
        scene.referenceBuilder.Attach( scene.referencePtr );
        for ( UInt32 i=0; i<3; ++i )
        {
            gl.context->InjectMouseMove( 400, 60 + (Int32) i );
            scene.reference->InjectMouseMove( 400, 60 + (Int32) i );
            gl.Frame();
            scene.reference->RenderFrame( 16 );
        }
        ASSERT_TRUE( SameDrawData( *gl.context->GetDrawData(), *scene.reference->GetDrawData() ) );
        IMAGE::TPixelMapPtr expected, actual;
        ASSERT_TRUE( gl.Capture( actual ) );
        ASSERT_TRUE( scene.reference->CaptureFrame( expected ) );
        UInt32 maxDifference = 0;
        const UInt32 perMille = DifferentPerMille( expected, actual, WindowTolerance, maxDifference );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Scripted window: " << perMille << " per mille of the pixels above the tolerance, the largest difference " << maxDifference );
        ASSERT_TRUE( perMille <= WindowMaxDifferentPerMille );
        ASSERT_TRUE( maxDifference < 64 );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: fill rule, scissor, nearest and linear sampling and straight alpha blending follow the rules of the software rasterizer" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        IMAGE::TPixelMapPtr ramp = MakeSolidImage( 2, 1, 0, 0, 0, 255 );
        UInt8* data = static_cast< UInt8* >( ramp->GetDataPtr() );
        data[ 4 ] = data[ 5 ] = data[ 6 ] = 255;
        gl.context->RegisterImage( "Ramp", ramp );
        CTestFrameBuilder builder( &BuildRasterizerProbes, GUCEF_NULL );
        builder.Attach( gl.contextPtr );
        gl.Frame( 2 );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( gl.Capture( pixels ) );
        if ( !pixels.IsNULL() )
        {
            UInt32 white = 0;
            for ( Int32 y=0; y<30; ++y )
                for ( Int32 x=0; x<30; ++x )
                    if ( Near( PixelAt( pixels, x, y ), White, 0 ) )
                        ++white;
            ASSERT_TRUE( 100 == white );
            ASSERT_TRUE( Near( PixelAt( pixels, 10, 10 ), White, 0 ) && Near( PixelAt( pixels, 19, 19 ), White, 0 ) );
            ASSERT_TRUE( Near( PixelAt( pixels, 9, 10 ), Black, 0 ) && Near( PixelAt( pixels, 20, 19 ), Black, 0 ) );

            UInt32 even = 0;
            for ( Int32 y=10; y<30; ++y )
                for ( Int32 x=30; x<50; ++x )
                    if ( Near( PixelAt( pixels, x, y ), GUI::CColour( 128, 0, 0, 255 ), 1 ) )
                        ++even;
            ASSERT_TRUE( 400 == even );

            UInt32 green = 0;
            for ( Int32 y=0; y<40; ++y )
                for ( Int32 x=50; x<90; ++x )
                    if ( Near( PixelAt( pixels, x, y ), Green, 0 ) )
                        ++green;
            ASSERT_TRUE( 100 == green );

            ASSERT_TRUE( 0 == PixelAt( pixels, 100, 10 ).r && 0 == PixelAt( pixels, 101, 10 ).r && 255 == PixelAt( pixels, 102, 10 ).r && 255 == PixelAt( pixels, 103, 10 ).r );
            ASSERT_TRUE( 0 == PixelAt( pixels, 100, 20 ).r && Near( PixelAt( pixels, 101, 20 ), GUI::CColour( 64, 64, 64, 255 ), 2 ) &&
                         Near( PixelAt( pixels, 102, 20 ), GUI::CColour( 191, 191, 191, 255 ), 2 ) && 255 == PixelAt( pixels, 103, 20 ).r );
            ASSERT_TRUE( Near( PixelAt( pixels, 125, 15 ), GUI::CColour( 128, 0, 0, 255 ), 1 ) );
        }
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: more vertices than 16-bit indices address are drawn, and a capture draws the last frame again without a new one" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        IMAGE::TPixelMapPtr none;
        ASSERT_FALSE( gl.Capture( none ) );
        CTestFrameBuilder builder( &BuildManyRects, GUCEF_NULL );
        builder.Attach( gl.contextPtr );
        gl.Frame();
        ImDrawData* drawData = gl.context->GetDrawData();
        ASSERT_TRUE( GUCEF_NULL != drawData && drawData->TotalVtxCount > 65536 );
        const UInt32 frames = gl.context->GetBuiltFrameCount();
        const UInt32 drawn = gl.context->GetDrawnFrameCount();
        IMAGE::TPixelMapPtr first, second;
        ASSERT_TRUE( gl.Capture( first ) );
        ASSERT_TRUE( gl.Capture( second ) );
        ASSERT_TRUE( frames == gl.context->GetBuiltFrameCount() && drawn == gl.context->GetDrawnFrameCount() );
        ASSERT_TRUE( 0 == CountDifferences( first, second, 0, GUI::CRect(), "two captures" ) );
        ASSERT_TRUE( Near( PixelAt( first, 1, 1 ), GUI::CColour( 0, 0, 255, 255 ), 0 ) );
        ASSERT_TRUE( Near( PixelAt( first, 597, 297 ), GUI::CColour( 0, 0, 255, 255 ), 0 ) );
        ASSERT_TRUE( Near( PixelAt( first, 599, 299 ), Black, 0 ) );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: the textures are made once, updated in place and destroyed with their image, with a GL error free context" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        CTestFrameBuilder builder( &BuildScriptedWindow, GUCEF_NULL );
        builder.Attach( gl.contextPtr );
        ASSERT_TRUE( gl.context->RegisterImage( "Checker", MakeSolidImage( 4, 4, 255, 255, 255, 255 ) ) );
        gl.Frame( 2 );
        CImGuiTextureCache& cache = gl.context->GetTextureCache();
        ASSERT_TRUE( 1 == cache.GetImageTextureCount() );
        IMAGE::TPixelMapPtr before, after;
        ASSERT_TRUE( gl.Capture( before ) );

        // new pixels in the same texture: the window shows them
        ASSERT_TRUE( RecolourImage( gl.context, "Checker", 0, 255, 0, 255 ) );
        gl.Frame( 2 );
        ASSERT_TRUE( 1 == cache.GetImageTextureCount() );
        ASSERT_TRUE( gl.Capture( after ) );
        ASSERT_TRUE( Near( PixelAt( before, 338, 173 ), White, 8 ) && Near( PixelAt( after, 338, 173 ), Green, 8 ) );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );

        // the image goes, its texture a frame or two later
        ASSERT_TRUE( gl.context->UnregisterImage( "Checker" ) );
        gl.Frame( 4 );
        ASSERT_TRUE( 0 == cache.GetImageTextureCount() && 0 == cache.GetPendingDestroyCount() );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );

        // an image again under the same name
        ASSERT_TRUE( gl.context->RegisterImage( "Checker", MakeSolidImage( 4, 4, 255, 0, 255, 255 ) ) );
        gl.Frame( 2 );
        ASSERT_TRUE( 1 == cache.GetImageTextureCount() );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );

        // an OpenGL texture of the application
        UInt32 name = 0;
        const GUIDRIVERIMGUIGL::CImGuiGLFunctions& functions = gl.window->GetFunctions();
        gl.window->MakeCurrent();
        functions.GenTextures( 1, &name );
        ASSERT_TRUE( 0 != name );
        ASSERT_TRUE( (ImTextureID) name == gl.context->GetTextureRef( name ).GetTexID() );
        functions.DeleteTextures( 1, &name );
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

    GUCEF_TESTFW_TESTCASE( "Render: a texture of the application is shown with ImGui::Image through its OpenGL name" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        const GUIDRIVERIMGUIGL::CImGuiGLFunctions& functions = gl.window->GetFunctions();
        UInt32 name = 0;
        functions.GenTextures( 1, &name );
        functions.BindTexture( GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_2D, name );
        const UInt8 blue[ 4 ] = { 0, 0, 255, 255 };
        functions.PixelStorei( GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_PACK_ALIGNMENT, 1 );
        functions.TexImage2D( GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_2D, 0, GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_RGBA8, 1, 1, 0,
                              GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_RGBA, GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_UNSIGNED_BYTE, blue );
        functions.TexParameteri( GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_2D, GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_MIN_FILTER, GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_NEAREST );
        functions.TexParameteri( GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_2D, GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_MAG_FILTER, GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_NEAREST );
        functions.BindTexture( GUIDRIVERIMGUIGL::CImGuiGLFunctions::GLC_TEXTURE_2D, 0 );

        struct SShow
        {
            static void Build( CImGuiContext& imguiContext, UInt32 deltaInMs, void* userData )
            {
                GUIDRIVERIMGUIGL::CImGuiOpenGLContext& glContext = static_cast< GUIDRIVERIMGUIGL::CImGuiOpenGLContext& >( imguiContext );
                ImDrawList* list = ImGui::GetForegroundDrawList();
                list->AddImage( glContext.GetTextureRef( *static_cast< UInt32* >( userData ) ), ImVec2( 200.0f, 200.0f ), ImVec2( 232.0f, 232.0f ) );
            }
        };
        CTestFrameBuilder builder( &SShow::Build, &name );
        builder.Attach( gl.contextPtr );
        gl.Frame();
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( gl.Capture( pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 210, 210 ), GUI::CColour( 0, 0, 255, 255 ), 1 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 231, 231 ), GUI::CColour( 0, 0, 255, 255 ), 1 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 232, 232 ), Black, 0 ) );
        functions.DeleteTextures( 1, &name );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: input through the Inject calls reaches the button: hover, click, and the colour of the button changes in the capture" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        SButtonState state;
        CTestFrameBuilder builder( &BuildButtonWindow, &state );
        builder.Attach( gl.contextPtr );
        gl.Frame( 2 );
        ASSERT_TRUE( state.maxX > state.minX && !state.hovered );
        const Int32 x = (Int32) ( ( state.minX + state.maxX ) / 2 );
        const Int32 y = (Int32) ( ( state.minY + state.maxY ) / 2 );
        IMAGE::TPixelMapPtr idle, hover;
        ASSERT_TRUE( gl.Capture( idle ) );

        gl.context->InjectMouseMove( x, y );
        gl.Frame( 2 );
        ASSERT_TRUE( state.hovered );
        ASSERT_TRUE( gl.Capture( hover ) );
        ASSERT_FALSE( Near( PixelAt( idle, x, y ), PixelAt( hover, x, y ), 8 ) );
        ASSERT_TRUE( gl.context->IsTakingPointerInput() );

        gl.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        gl.Frame();
        gl.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        gl.Frame( 2 );
        ASSERT_TRUE( 1 == state.clicks );

        gl.context->InjectMouseMove( 600, 400 );
        gl.Frame( 2 );
        ASSERT_FALSE( state.hovered );
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

    GUCEF_TESTFW_TESTCASE( "Render: a window of another size shows the picture where the present mode says, the fills have the pixels of the picture, and a resize keeps it so" )
    try
    {
        // each window pixel shows the picture pixel at its centre: blocks when scaled up, black around the picture
        //
        // At the fractional scale of the second case the scissor rectangle of the upstream backend, which cuts the
        // size of the clip rectangle to whole pixels, takes the first or the last row or column of a clipped fill
        // when the clip edge lies inside a pixel: that case allows a few pixels along the edges of clipped panels
        struct SCase { UInt32 windowWidth; UInt32 windowHeight; GUI::TPresentMode mode; UInt32 maxWrongPerMille; };
        const SCase cases[] = { { 1280, 960, GUI::PRESENTMODE_NEAREST, 0 }, { 1000, 600, GUI::PRESENTMODE_NEAREST, 2 }, { 1000, 600, GUI::PRESENTMODE_INTEGER, 0 } };
        for ( size_t c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
        {
            SGlTestWindow gl;
            ASSERT_TRUE( gl.Make( cases[ c ].windowWidth, cases[ c ].windowHeight, false ) );
            if ( GUCEF_NULL == gl.context )
                continue;
            gl.context->SetPresentMode( cases[ c ].mode );
            BuildFillScene( gl.context );
            gl.Frame();

            IMAGE::TPixelMapPtr picture, presented;
            ASSERT_TRUE( gl.Capture( picture ) );
            ASSERT_TRUE( gl.window->MakeCurrent() && gl.context->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( !presented.IsNULL() && cases[ c ].windowWidth == presented->GetWidthInPixels() && cases[ c ].windowHeight == presented->GetHeightInPixels() );
            if ( picture.IsNULL() || presented.IsNULL() )
                continue;

            const GUI::CRect area = gl.context->GetPresentRect();
            UInt32 wrong = 0;
            for ( Int32 y=0; y<(Int32) cases[ c ].windowHeight; ++y )
            {
                for ( Int32 x=0; x<(Int32) cases[ c ].windowWidth; ++x )
                {
                    const GUI::CColour actual = PixelAt( presented, x, y );
                    if ( x < area.left || x >= area.Right() || y < area.top || y >= area.Bottom() )
                    {
                        if ( !Near( actual, Black, 2 ) )
                            ++wrong;
                        continue;
                    }

                    // a pixel centre exactly on the edge of two picture pixels may show either of them
                    const double fx = ( ( x - area.left ) + 0.5 ) * 640.0 / area.width;
                    const double fy = ( ( y - area.top ) + 0.5 ) * 480.0 / area.height;
                    const Int32 px = (Int32) fx;
                    const Int32 py = (Int32) fy;
                    const bool edgeX = px > 0 && fx - px < 1e-6;
                    const bool edgeY = py > 0 && fy - py < 1e-6;
                    bool match = Near( actual, PixelAt( picture, px, py ), RetainedTolerance );
                    if ( !match && edgeX )
                        match = Near( actual, PixelAt( picture, px - 1, py ), RetainedTolerance );
                    if ( !match && edgeY )
                        match = Near( actual, PixelAt( picture, px, py - 1 ), RetainedTolerance );
                    if ( !match && edgeX && edgeY )
                        match = Near( actual, PixelAt( picture, px - 1, py - 1 ), RetainedTolerance );
                    if ( !match )
                    {
                        if ( ++wrong <= 6 )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Present case " << (UInt32) c << ": pixel " << x << "," << y << " is " << actual.ToString()
                                << ", the picture has " << PixelAt( picture, px, py ).ToString() << " at " << px << "," << py );
                    }
                }
            }
            if ( 0 != wrong )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Present case " << (UInt32) c << ": " << wrong << " pixels differ" );
            ASSERT_TRUE( wrong * 1000 <= cases[ c ].maxWrongPerMille * cases[ c ].windowWidth * cases[ c ].windowHeight );
            ASSERT_TRUE( 0 == gl.window->TakeGlError() );
        }

        // the window grows: the same picture, shown over the new size
        SGlTestWindow grow;
        ASSERT_TRUE( grow.Make( 640, 480, false ) );
        if ( GUCEF_NULL != grow.context )
        {
            BuildFillScene( grow.context );
            grow.Frame();
            IMAGE::TPixelMapPtr small, big, picture;
            ASSERT_TRUE( grow.window->MakeCurrent() && grow.context->CapturePresentedFrame( small ) );
            ASSERT_TRUE( grow.window->Resize( 1280, 960 ) );
            grow.Frame();
            ASSERT_TRUE( grow.window->MakeCurrent() && grow.context->CapturePresentedFrame( big ) );
            ASSERT_TRUE( grow.Capture( picture ) );
            ASSERT_TRUE( !big.IsNULL() && 1280 == big->GetWidthInPixels() && 960 == big->GetHeightInPixels() && 640 == picture->GetWidthInPixels() );
            UInt32 wrong = 0;
            for ( Int32 y=0; y<480; y+=7 )
                for ( Int32 x=0; x<640; x+=5 )
                    if ( !Near( PixelAt( big, x * 2, y * 2 ), PixelAt( picture, x, y ), RetainedTolerance ) || !Near( PixelAt( big, x * 2 + 1, y * 2 + 1 ), PixelAt( picture, x, y ), RetainedTolerance ) )
                        ++wrong;
            ASSERT_TRUE( 0 == wrong );
            ASSERT_TRUE( 0 == grow.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: the layer above is blended over the one below in the same window, the lowest clears it, and the layer numbers decide the order" )
    try
    {
        SGlTestWindow gl;
        ASSERT_TRUE( gl.Make( 640, 480, false ) );
        if ( GUCEF_NULL == gl.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        gl.window->MakeCurrent();
        GUI::TGuiContextPtr topPtr = GetOpenGLDriver().CreateGUIContext( gl.windowPtr );
        CImGuiContext* top = AsImGui( topPtr );
        ASSERT_TRUE( GUCEF_NULL != top );
        if ( GUCEF_NULL == top )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        top->SetLogicalResolution( 640, 480 );
        gl.context->SetLayer( 1 );
        top->SetLayer( 2 );
        ASSERT_TRUE( gl.context->IsBottomLayer() && !top->IsBottomLayer() );

        GUI::CWidget* red = Screen( gl.context );
        red->SetColourProperty( "BackgroundColour", Red );
        SButtonState state;
        CTestFrameBuilder builder( &BuildButtonWindow, &state );
        builder.Attach( topPtr );

        gl.Frame();
        gl.context->RenderWindowLayers( 16 );
        IMAGE::TPixelMapPtr presented;
        ASSERT_TRUE( top->CapturePresentedFrame( presented ) );
        ASSERT_TRUE( Near( PixelAt( presented, 5, 5 ), Red, 2 ) );
        ASSERT_TRUE( Near( PixelAt( presented, 600, 400 ), Red, 2 ) );
        // inside the window of Dear ImGui (20,20 to 220,120): its dark background over the red, which does not show through
        ASSERT_FALSE( Near( PixelAt( presented, 100, 100 ), Red, 40 ) );

        // the layer numbers decide the order, not the order of making: the red context is above now
        gl.context->SetLayer( 3 );
        ASSERT_TRUE( top->IsBottomLayer() && !gl.context->IsBottomLayer() );
        top->RenderWindowLayers( 16 );
        ASSERT_TRUE( gl.context->CapturePresentedFrame( presented ) );
        ASSERT_TRUE( Near( PixelAt( presented, 100, 100 ), Red, 2 ) );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: a context of the driver in a second window keeps drawing when the first one goes, and a context made again in the same window has the same pixels" )
    try
    {
        SGlTestWindow first, second;
        ASSERT_TRUE( first.Make( 640, 480, false ) );
        ASSERT_TRUE( second.Make( 640, 480, false ) );
        if ( GUCEF_NULL == first.context || GUCEF_NULL == second.context )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        BuildMixedScene( first.context );
        BuildMixedScene( second.context );
        first.Frame();
        second.Frame();
        IMAGE::TPixelMapPtr expected, captured;
        ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *second.context, expected ) );

        // the backend of the first context shuts down; its entry points are gone for the second one unless it is fed again
        first.DestroyContext();
        second.Frame();
        ASSERT_TRUE( second.Capture( captured ) );
        ASSERT_TRUE( 0 == CountDifferences( expected, captured, RetainedTolerance, GUI::CRect(), "after the other context went" ) );
        ASSERT_TRUE( 0 == second.window->TakeGlError() );

        // made again
        ASSERT_TRUE( first.MakeContext() );
        BuildMixedScene( first.context );
        first.Frame();
        IMAGE::TPixelMapPtr again;
        ASSERT_TRUE( first.Capture( again ) );
        ASSERT_TRUE( 0 == CountDifferences( expected, again, RetainedTolerance, GUI::CRect(), "a context made again" ) );
        ASSERT_TRUE( 0 == first.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: with OpenGL ES 3 (what a Wayland window falls back to) the shaders are the ones of GLSL ES and the pixels are those of the reference" )
    try
    {
        SGlTestWindow gl;
        if ( !gl.Make( 640, 480, true ) )
            GUCEF_TESTFW_SKIP( "no OpenGL ES 3 context can be made on this machine" );
        ASSERT_TRUE( "#version 300 es" == gl.context->GetGlslVersion() );
        SScriptedScene scene( &BuildScriptedWindow );
        ASSERT_TRUE( GUCEF_NULL != scene.reference );
        if ( GUCEF_NULL == scene.reference )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        BuildMixedScene( gl.context );
        BuildMixedScene( scene.reference );
        scene.glBuilder.Attach( gl.contextPtr );
        scene.referenceBuilder.Attach( scene.referencePtr );
        for ( UInt32 i=0; i<2; ++i )
        {
            gl.Frame();
            scene.reference->RenderFrame( 16 );
        }
        IMAGE::TPixelMapPtr expected, actual;
        ASSERT_TRUE( gl.Capture( actual ) );
        ASSERT_TRUE( scene.reference->CaptureFrame( expected ) );
        UInt32 maxDifference = 0;
        const UInt32 perMille = DifferentPerMille( expected, actual, WindowTolerance, maxDifference );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OpenGL ES scripted window: " << perMille << " per mille of the pixels above the tolerance, the largest difference " << maxDifference );
        ASSERT_TRUE( perMille <= WindowMaxDifferentPerMille );
        ASSERT_TRUE( 0 == gl.window->TakeGlError() );
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

    GUCEF_TESTFW_TESTCASE( "Render: contexts that go before their window leave no error entry and no GL error behind" )
    try
    {
        CLogTextCounter contextErrors( "ImGuiOpenGLContext:" );
        CLogTextCounter framebufferErrors( "ImGuiGLFramebuffer:" );
        CLogTextCounter imguiErrors( "ImGui: " );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &contextErrors );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &framebufferErrors );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &imguiErrors );
        {
            SGlTestWindow gl;
            ASSERT_TRUE( gl.Make( 640, 480, false ) );
            if ( GUCEF_NULL != gl.context )
            {
                BuildMixedScene( gl.context );
                CTestFrameBuilder builder( &BuildScriptedWindow, GUCEF_NULL );
                builder.Attach( gl.contextPtr );
                gl.Frame( 3 );
                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( gl.Capture( pixels ) );
                ASSERT_TRUE( 0 == gl.window->TakeGlError() );
                gl.DestroyContext();
                ASSERT_TRUE( 0 == gl.window->TakeGlError() );
            }
        }
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &contextErrors );
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &framebufferErrors );
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &imguiErrors );
        ASSERT_TRUE( 0 == contextErrors.m_count + framebufferErrors.m_count + imguiErrors.m_count );
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

#else

void
PerformGLRenderTests( void )
{
}

#endif /* linux ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
