/*
 *  guidriverImGuiOgre_TestApp: tests of the gucefGUI driver ImGuiOgre
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

#include "imgui.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGuiOgre_TestSupport.h"
#include "TestImGuiOgreScenes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIOGRETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUIDRIVERIMGUI;
using namespace GUIDRIVERIMGUIOGRE;

namespace {

/*
 *  Tolerance policy of the captures, as the RmlUi render tests of gucefGUI_TestApp use it: what is drawn with rectangles
 *  of the retained widgets (fills, strokes, images, text quads) equals the CSoftwareRenderer reference within
 *  RetainedTolerance per channel and without a different pixel; what Dear ImGui draws itself (rounded corners,
 *  anti-aliased lines, glyph edges) has the same triangles as the reference and differs by the rounding of the
 *  interpolation of the GPU: within WindowTolerance per channel, and WindowMaxDifferentPerMille of a thousand pixels
 *  may exceed it.
 */
const Int32 RetainedTolerance = 2;
const Int32 WindowTolerance = 3;
const UInt32 WindowMaxDifferentPerMille = 5;

CImGuiOgreContext*
AsOgre( const GUI::TGuiContextPtr& context )
{
    return static_cast< CImGuiOgreContext* >( AsImGui( context ) );
}

/** The Ogre window of the tests, or NULL, with the reason for the skip notice in the log */
bool
HaveWindow( void )
{
    return TESTWINDOWMODE_NONE != GetTestWindowMode() && !GetMainTestWindow().IsNULL() && LoadFreeTypeFontPlugin();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformImGuiOgreRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Render" );

    if ( TESTWINDOWMODE_NONE == GetTestWindowMode() )
    {
        GUCEF_TESTFW_TESTCASE( "Render: windows for the rendering tests" )
        GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        GUCEF_TESTFW_TESTCASE_END
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Render: a retained scene of every kind of drawing, captured from the overlay, has the pixels of the software reference" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            BuildMixedScene( context );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr expected, actual;
            ASSERT_TRUE( context->CaptureFrame( actual ) );
            ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *context, expected ) );
            UInt32 maxDifference = 0;
            const UInt32 perMille = DifferentPerMille( expected, actual, RetainedTolerance, maxDifference );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Mixed scene: " << perMille << " per mille of the pixels above the tolerance, the largest difference " << maxDifference );
            ASSERT_TRUE( 0 == CountDifferences( expected, actual, RetainedTolerance, GUI::CRect(), "mixed scene" ) );
            ASSERT_TRUE( context->GetRenderer()->GetDrawCallCount() > 0 );
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

    GUCEF_TESTFW_TESTCASE( "Render: at a render scale of 2 the capture has the render size and pictures and fonts of density 2 have the pixels of the reference" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            BuildDensityScene( context );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr expected, actual;
            ASSERT_TRUE( context->CaptureFrame( actual ) );
            ASSERT_TRUE( !actual.IsNULL() && 1280 == actual->GetWidthInPixels() && 960 == actual->GetHeightInPixels() );
            ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *context, expected ) );
            // the line of the draw surface: the reference walks it in pixels of the render scale, the painter in logical pixels
            ASSERT_TRUE( 0 == CountDifferences( expected, actual, RetainedTolerance, GUI::CRect( 240, 20, 80, 80 ), "density scene" ) );
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

    GUCEF_TESTFW_TESTCASE( "Render: a scripted window of Dear ImGui next to the retained scene has the triangles of the software reference, within the tolerance of the GPU" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        GUI::TGuiContextPtr referencePtr = CreateSoftwareContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        CImGuiContext* reference = AsImGui( referencePtr );
        ASSERT_TRUE( GUCEF_NULL != context && GUCEF_NULL != reference );
        if ( GUCEF_NULL != context && GUCEF_NULL != reference )
        {
            BuildMixedScene( context );
            BuildMixedScene( reference );
            CTestFrameBuilder builder( &BuildScriptedWindow, GUCEF_NULL );
            CTestFrameBuilder referenceBuilder( &BuildScriptedWindow, GUCEF_NULL );
            builder.Attach( contextPtr );
            referenceBuilder.Attach( referencePtr );
            for ( UInt32 i=0; i<3; ++i )
            {
                context->InjectMouseMove( 400, 60 + (Int32) i );
                reference->InjectMouseMove( 400, 60 + (Int32) i );
                context->RenderFrame( 16 );
                reference->RenderFrame( 16 );
            }
            ASSERT_TRUE( SameDrawData( *context->GetDrawData(), *reference->GetDrawData() ) );
            IMAGE::TPixelMapPtr expected, actual;
            ASSERT_TRUE( context->CaptureFrame( actual ) );
            ASSERT_TRUE( reference->CaptureFrame( expected ) );
            UInt32 maxDifference = 0;
            const UInt32 perMille = DifferentPerMille( expected, actual, WindowTolerance, maxDifference );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Scripted window: " << perMille << " per mille of the pixels above the tolerance, the largest difference " << maxDifference );
            ASSERT_TRUE( perMille <= WindowMaxDifferentPerMille );
            ASSERT_TRUE( maxDifference < 64 );
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

    GUCEF_TESTFW_TESTCASE( "Render: fill rule, scissor, nearest and linear sampling and straight alpha blending follow the rules of the software rasterizer" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            IMAGE::TPixelMapPtr ramp = MakeSolidImage( 2, 1, 0, 0, 0, 255 );
            UInt8* data = static_cast< UInt8* >( ramp->GetDataPtr() );
            data[ 4 ] = data[ 5 ] = data[ 6 ] = 255;
            context->RegisterImage( "Ramp", ramp );
            CTestFrameBuilder builder( &BuildRasterizerProbes, GUCEF_NULL );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( context->CaptureFrame( pixels ) );
            if ( !pixels.IsNULL() )
            {
                // fill rule: 10 x 10 pixels from 10,10
                UInt32 white = 0;
                for ( Int32 y=0; y<30; ++y )
                    for ( Int32 x=0; x<30; ++x )
                        if ( Near( PixelAt( pixels, x, y ), White, 0 ) )
                            ++white;
                ASSERT_TRUE( 100 == white );
                ASSERT_TRUE( Near( PixelAt( pixels, 10, 10 ), White, 0 ) && Near( PixelAt( pixels, 19, 19 ), White, 0 ) );
                ASSERT_TRUE( Near( PixelAt( pixels, 9, 10 ), Black, 0 ) && Near( PixelAt( pixels, 20, 19 ), Black, 0 ) );

                // the shared edge of two triangles is drawn once: every pixel of the quad has the same colour
                UInt32 even = 0;
                for ( Int32 y=10; y<30; ++y )
                    for ( Int32 x=30; x<50; ++x )
                        if ( Near( PixelAt( pixels, x, y ), GUI::CColour( 128, 0, 0, 255 ), 1 ) )
                            ++even;
                ASSERT_TRUE( 400 == even );

                // scissor: only the 10 x 10 pixels of the clip rectangle
                UInt32 green = 0;
                for ( Int32 y=0; y<40; ++y )
                    for ( Int32 x=50; x<90; ++x )
                        if ( Near( PixelAt( pixels, x, y ), Green, 0 ) )
                            ++green;
                ASSERT_TRUE( 100 == green );

                // nearest and linear sampling of a black and a white texel over four pixels
                ASSERT_TRUE( 0 == PixelAt( pixels, 100, 10 ).r && 0 == PixelAt( pixels, 101, 10 ).r && 255 == PixelAt( pixels, 102, 10 ).r && 255 == PixelAt( pixels, 103, 10 ).r );
                ASSERT_TRUE( 0 == PixelAt( pixels, 100, 20 ).r && Near( PixelAt( pixels, 101, 20 ), GUI::CColour( 64, 64, 64, 255 ), 2 ) &&
                             Near( PixelAt( pixels, 102, 20 ), GUI::CColour( 191, 191, 191, 255 ), 2 ) && 255 == PixelAt( pixels, 103, 20 ).r );

                // half red over black
                ASSERT_TRUE( Near( PixelAt( pixels, 125, 15 ), GUI::CColour( 128, 0, 0, 255 ), 1 ) );
            }
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

    GUCEF_TESTFW_TESTCASE( "Render: more vertices than 16-bit indices address are drawn, and a capture draws the last frame again without a new one" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            IMAGE::TPixelMapPtr none;
            ASSERT_FALSE( context->CaptureFrame( none ) );
            CTestFrameBuilder builder( &BuildManyRects, GUCEF_NULL );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            ImDrawData* drawData = context->GetDrawData();
            ASSERT_TRUE( GUCEF_NULL != drawData && drawData->TotalVtxCount > 65536 );
            const UInt32 frames = context->GetBuiltFrameCount();
            IMAGE::TPixelMapPtr first, second;
            ASSERT_TRUE( context->CaptureFrame( first ) );
            ASSERT_TRUE( context->CaptureFrame( second ) );
            ASSERT_TRUE( frames == context->GetBuiltFrameCount() );
            ASSERT_TRUE( 0 == CountDifferences( first, second, 0, GUI::CRect(), "two captures" ) );
            ASSERT_TRUE( context->GetRenderer()->GetVertexBufferCapacity() >= (UInt32) drawData->TotalVtxCount );
            // the first and the last rectangle (the vertices of the last ones are past 65,535)
            ASSERT_TRUE( Near( PixelAt( first, 1, 1 ), GUI::CColour( 0, 0, 255, 255 ), 0 ) );
            ASSERT_TRUE( Near( PixelAt( first, 597, 297 ), GUI::CColour( 0, 0, 255, 255 ), 0 ) );
            ASSERT_TRUE( Near( PixelAt( first, 599, 299 ), Black, 0 ) );
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

    GUCEF_TESTFW_TESTCASE( "Render: the textures of Dear ImGui are Ogre textures that are made once, updated in place and destroyed with their image, and none is left when the context goes" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        const UInt32 before = CountImGuiOgreTextures();
        {
            GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
            CImGuiOgreContext* context = AsOgre( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
            CImGuiOgreRenderer* renderer = context->GetRenderer();

            // the font atlas
            context->RenderFrame( 16 );
            ASSERT_TRUE( before + 1 == CountImGuiOgreTextures() );
            ASSERT_TRUE( 1 == renderer->GetTexturesCreated() );

            // an image of the context, shown with ImGui::Image
            ASSERT_TRUE( context->RegisterImage( "Pic", MakeSolidImage( 8, 8, 255, 0, 0, 255 ) ) );
            CTestFrameBuilder builder( &BuildScriptedWindow, GUCEF_NULL );
            context->RegisterImage( "Checker", MakeSolidImage( 4, 4, 255, 255, 255, 255 ) );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( before + 2 == CountImGuiOgreTextures() );
            ASSERT_TRUE( 2 == renderer->GetTexturesCreated() );
            const UInt32 updates = renderer->GetTexturesUpdated();

            // new pixels in the same texture
            ASSERT_TRUE( RecolourImage( context, "Checker", 0, 255, 0, 255 ) );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 2 == renderer->GetTexturesCreated() );
            ASSERT_TRUE( before + 2 == CountImGuiOgreTextures() );
            ASSERT_TRUE( renderer->GetTexturesUpdated() > updates );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( context->CaptureFrame( pixels ) );

            // the image goes, its texture goes a frame or two later
            ASSERT_TRUE( context->UnregisterImage( "Checker" ) );
            for ( UInt32 i=0; i<4; ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( before + 1 == CountImGuiOgreTextures() );
            ASSERT_TRUE( renderer->GetTexturesDestroyed() >= 1 );
            ASSERT_TRUE( 0 == context->GetTextureCache().GetPendingDestroyCount() );
        }
        // the context took its textures with it
        ASSERT_TRUE( before == CountImGuiOgreTextures() );
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

    GUCEF_TESTFW_TESTCASE( "Render: an Ogre texture of the application is shown with ImGui::Image through its resource handle" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            Ogre::TexturePtr texture = MakeSolidTexture( "ImGuiOgreTest/Blue", 16, 16, 0, 0, 255, 255 );
            struct SShow
            {
                static void Build( CImGuiContext& imguiContext, UInt32 deltaInMs, void* userData )
                {
                    const ImTextureRef* reference = static_cast< const ImTextureRef* >( userData );
                    ImGui::GetForegroundDrawList()->AddImage( *reference, ImVec2( 200.0f, 200.0f ), ImVec2( 232.0f, 232.0f ) );
                }
            };
            ImTextureRef reference = context->GetTextureRef( texture );
            CTestFrameBuilder builder( &SShow::Build, &reference );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( context->CaptureFrame( pixels ) );
            ASSERT_TRUE( Near( PixelAt( pixels, 210, 210 ), GUI::CColour( 0, 0, 255, 255 ), 1 ) );
            ASSERT_TRUE( Near( PixelAt( pixels, 231, 231 ), GUI::CColour( 0, 0, 255, 255 ), 1 ) );
            ASSERT_TRUE( Near( PixelAt( pixels, 232, 232 ), Black, 0 ) );
            ASSERT_TRUE( 0 == CountImGuiOgreTextures() - 1 );

            // the application removes its texture: the draw command is skipped and nothing else breaks
            Ogre::TextureManager::getSingleton().remove( texture );
            texture.reset();
            context->RenderFrame( 16 );
            ASSERT_TRUE( context->CaptureFrame( pixels ) );
            ASSERT_TRUE( Near( PixelAt( pixels, 210, 210 ), Black, 0 ) );
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

    GUCEF_TESTFW_TESTCASE( "Render: input through the Inject calls reaches the button: hover, click, and the colour of the button changes in the capture" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SButtonState state;
            CTestFrameBuilder builder( &BuildButtonWindow, &state );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( state.maxX > state.minX && !state.hovered );
            const Int32 x = (Int32) ( ( state.minX + state.maxX ) / 2 );
            const Int32 y = (Int32) ( ( state.minY + state.maxY ) / 2 );
            IMAGE::TPixelMapPtr idle, hover;
            ASSERT_TRUE( context->CaptureFrame( idle ) );

            context->InjectMouseMove( x, y );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( state.hovered );
            ASSERT_TRUE( context->CaptureFrame( hover ) );
            ASSERT_FALSE( Near( PixelAt( idle, x, y ), PixelAt( hover, x, y ), 8 ) );
            ASSERT_TRUE( context->IsTakingPointerInput() );

            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->RenderFrame( 16 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 1 == state.clicks );

            context->InjectMouseMove( 600, 400 );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_FALSE( state.hovered );
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

    GUCEF_TESTFW_TESTCASE( "Render: the window draws the overlay in its frame, and the presented frame has what the capture has" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
        CImGuiOgreContext* context = AsOgre( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            BuildFillScene( context );
            const UInt32 drawn = context->GetOverlayDrawCount();
            IMAGE::TPixelMapPtr presented, captured;
            ASSERT_TRUE( context->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( drawn + 1 == context->GetOverlayDrawCount() );
            ASSERT_TRUE( context->CaptureFrame( captured ) );
            ASSERT_TRUE( drawn + 1 == context->GetOverlayDrawCount() );
            ASSERT_TRUE( !presented.IsNULL() && 640 == presented->GetWidthInPixels() && 480 == presented->GetHeightInPixels() );
            ASSERT_TRUE( 0 == CountDifferences( captured, presented, RetainedTolerance, GUI::CRect(), "presented against captured" ) );
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

    GUCEF_TESTFW_TESTCASE( "Render: a window of another size shows the picture where the present mode says, and the fills have the pixels of the picture" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );

        // each window pixel shows the picture pixel at its centre: blocks when scaled up, black around the picture
        struct SCase { UInt32 windowWidth; UInt32 windowHeight; GUI::TPresentMode mode; };
        const SCase cases[] = { { 1280, 960, GUI::PRESENTMODE_NEAREST }, { 1000, 600, GUI::PRESENTMODE_NEAREST }, { 1000, 600, GUI::PRESENTMODE_INTEGER } };
        for ( size_t c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
        {
            GUI::TWindowContextPtr window = MakeTestOgreWindow( cases[ c ].windowWidth, cases[ c ].windowHeight );
            ASSERT_TRUE( !window.IsNULL() );
            if ( window.IsNULL() )
                continue;
            GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext( window );
            CImGuiOgreContext* context = AsOgre( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                continue;
            context->SetPresentMode( cases[ c ].mode );
            BuildFillScene( context );
            context->RenderFrame( 16 );

            IMAGE::TPixelMapPtr picture, presented;
            ASSERT_TRUE( context->CaptureFrame( picture ) );
            ASSERT_TRUE( context->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( !presented.IsNULL() && cases[ c ].windowWidth == presented->GetWidthInPixels() && cases[ c ].windowHeight == presented->GetHeightInPixels() );
            if ( picture.IsNULL() || presented.IsNULL() )
                continue;

            const GUI::CRect area = context->GetPresentRect();
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
            ASSERT_TRUE( 0 == wrong );

            // the window closes with its context first
            contextPtr = GUI::TGuiContextPtr();
            GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );
            if ( !backend.IsNULL() )
                backend->DestroyWindowContext( window );
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

    GUCEF_TESTFW_TESTCASE( "Render: a higher layer of another renderer family member is drawn over the lower one in the same window, in the order of the layers" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
        if ( GUCEF_NULL == manager.GetGuiDriver( "RmlUiOgre" ) )
            GUCEF_TESTFW_SKIP( "the RmlUiOgre driver is not there" );
        GUI::TWindowContextPtr window = GetMainTestWindow();

        // RmlUiOgre below and ImGuiOgre above: the window of Dear ImGui is over the red of RmlUi, the red shows around it
        {
            GUI::TGuiContextPtr rmlPtr = manager.CreateGUIContext( "RmlUiOgre", window );
            GUI::CGUIContext* rml = static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() );
            GUI::TGuiContextPtr imguiPtr = CreateImGuiOgreContext( window );
            CImGuiOgreContext* imgui = AsOgre( imguiPtr );
            ASSERT_TRUE( GUCEF_NULL != rml && GUCEF_NULL != imgui );
            if ( GUCEF_NULL != rml && GUCEF_NULL != imgui )
            {
                rml->SetLogicalResolution( 640, 480 );
                rml->SetLayer( 1 );
                imgui->SetLayer( 2 );
                ASSERT_TRUE( rml->IsBottomLayer() && !imgui->IsBottomLayer() );
                GUI::CWidget* red = Screen( rml );
                red->SetColourProperty( "BackgroundColour", Red );
                SButtonState state;
                CTestFrameBuilder builder( &BuildButtonWindow, &state );
                builder.Attach( imguiPtr );

                rml->RenderWindowLayers( 16 );
                IMAGE::TPixelMapPtr presented;
                ASSERT_TRUE( imgui->CapturePresentedFrame( presented ) );
                ASSERT_TRUE( Near( PixelAt( presented, 5, 5 ), Red, 2 ) );
                ASSERT_TRUE( Near( PixelAt( presented, 600, 400 ), Red, 2 ) );
                // inside the window of Dear ImGui (20,20 to 220,120): its dark background, not the red
                ASSERT_FALSE( Near( PixelAt( presented, 100, 100 ), Red, 40 ) );

                // the layer numbers decide the order, not the order of making
                imgui->SetLayer( 0 );
                rml->SetLayer( 3 );
                ASSERT_TRUE( imgui->IsBottomLayer() && !rml->IsBottomLayer() );
                imgui->RenderWindowLayers( 16 );
                ASSERT_TRUE( rml->CapturePresentedFrame( presented ) );
                // RmlUi is above now: its red fills the window over the window of Dear ImGui
                ASSERT_TRUE( Near( PixelAt( presented, 100, 100 ), Red, 2 ) );
            }
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

    GUCEF_TESTFW_TESTCASE( "Render: a context that goes, and one made in its place, work without an error entry, and nothing of the first is left in Ogre" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( "no Ogre window can be made" );
        CLogTextCounter counter( "ImGui" );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &counter );
        IMAGE::TPixelMapPtr first, second;
        const UInt32 before = CountImGuiOgreTextures();
        for ( UInt32 round=0; round<2; ++round )
        {
            GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
            CImGuiOgreContext* context = AsOgre( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                break;
            BuildMixedScene( context );
            context->RenderFrame( 16 );
            ASSERT_TRUE( context->CaptureFrame( 0 == round ? first : second ) );
        }
        ASSERT_TRUE( before == CountImGuiOgreTextures() );
        ASSERT_TRUE( 0 == CountDifferences( first, second, 0, GUI::CRect(), "a context made again" ) );
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &counter );
        ASSERT_TRUE( 0 == counter.m_count );
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIOGRETEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
