/*
 *  guidriverImGui_TestApp: tests of the gucefGUI driver library of Dear ImGui
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
#include "imgui_internal.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGui_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUITEST {

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

/** Images an ImGui window shows, with the size each got */
struct SImageWindow
{
    GUCEF::vector< CString > references;
    GUCEF::vector< ImVec2 > sizes;
    GUCEF::vector< bool > shown;
};

void
BuildImageWindow( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SImageWindow* window = static_cast< SImageWindow* >( userData );
    window->sizes.clear();
    window->shown.clear();
    ImGui::SetNextWindowPos( ImVec2( 10.0f, 10.0f ), ImGuiCond_Always );
    ImGui::Begin( "Images" );
    for ( UInt32 i=0; i<(UInt32)window->references.size(); ++i )
    {
        bool shown = context.Image( window->references[ i ] );
        window->shown.push_back( shown );
        window->sizes.push_back( shown ? ImGui::GetItemRectSize() : ImVec2( 0.0f, 0.0f ) );
    }
    ImGui::End();
}

}

/*-------------------------------------------------------------------------*/

void
PerformImGuiTextureTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Textures" );

    GUCEF_TESTFW_TESTCASE( "Textures: a registered image is made a texture once, an update updates it once, unregistering it destroys it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            CImGuiSoftwareRenderer& renderer = context->GetSoftwareRenderer();
            CImGuiTextureCache& cache = context->GetTextureCache();
            SImageWindow window;
            CTestFrameBuilder builder( &BuildImageWindow, &window );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );

            ASSERT_TRUE( context->RegisterImage( "Picture", MakeSolidImage( 8, 4, 10, 20, 30, 255 ) ) );
            window.references.push_back( "Picture" );
            UInt32 creates = renderer.GetTextureCreateCount();
            UInt32 updates = renderer.GetTextureUpdateCount();
            UInt32 destroys = renderer.GetTextureDestroyCount();
            for ( UInt32 i=0; i<3; ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( creates + 1 == renderer.GetTextureCreateCount() );
            ASSERT_TRUE( updates == renderer.GetTextureUpdateCount() );
            ASSERT_TRUE( 1 == cache.GetImageTextureCount() );
            ASSERT_TRUE( window.shown.size() == 1 && window.shown[ 0 ] );

            // new pixels of the same size: one update of the whole texture
            IMAGE::TPixelMapPtr pixels;
            UInt32 revision = 0;
            ASSERT_TRUE( context->ResolveImage( "Picture", pixels, revision ) );
            static_cast< UInt8* >( pixels->GetDataPtr() )[ 0 ] = 200;
            ASSERT_TRUE( context->UpdateImage( "Picture" ) );
            for ( UInt32 i=0; i<3; ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( creates + 1 == renderer.GetTextureCreateCount() );
            ASSERT_TRUE( updates + 1 == renderer.GetTextureUpdateCount() );
            ImTextureRef reference = context->GetImageTextureRef( "Picture" );
            ASSERT_TRUE( GUCEF_NULL != reference._TexData && 200 == reference._TexData->Pixels[ 0 ] );

            // unregistered: the texture goes, through a destroy request of Dear ImGui
            window.references.clear();
            ASSERT_TRUE( context->UnregisterImage( "Picture" ) );
            ASSERT_TRUE( 0 == cache.GetImageTextureCount() );
            ASSERT_TRUE( 1 == cache.GetPendingDestroyCount() );
            for ( UInt32 i=0; i<3; ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( destroys + 1 == renderer.GetTextureDestroyCount() );
            ASSERT_TRUE( 0 == cache.GetPendingDestroyCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures: an image is shown at its logical size, its pixels divided by its density; an RGB image becomes an RGBA texture" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( context->RegisterImage( "Fine", MakeSolidImage( 16, 8, 1, 2, 3, 255 ), 2 ) );
            IMAGE::TPixelMapPtr rgb( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 3, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
            UInt8* data = static_cast< UInt8* >( rgb->GetDataPtr() );
            for ( UInt32 i=0; i<6; ++i )
            {
                data[ i*3 ] = 100;
                data[ i*3+1 ] = 150;
                data[ i*3+2 ] = 200;
            }
            ASSERT_TRUE( context->RegisterImage( "Rgb", rgb ) );

            SImageWindow window;
            window.references.push_back( "Fine" );
            window.references.push_back( "Rgb" );
            window.references.push_back( "NoSuchImage" );
            CTestFrameBuilder builder( &BuildImageWindow, &window );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 3 == window.shown.size() );
            if ( 3 == window.shown.size() )
            {
                ASSERT_TRUE( window.shown[ 0 ] && 8.0f == window.sizes[ 0 ].x && 4.0f == window.sizes[ 0 ].y );
                ASSERT_TRUE( window.shown[ 1 ] && 3.0f == window.sizes[ 1 ].x && 2.0f == window.sizes[ 1 ].y );
                ASSERT_FALSE( window.shown[ 2 ] );
            }
            ImTextureRef reference = context->GetImageTextureRef( "Rgb" );
            ASSERT_TRUE( GUCEF_NULL != reference._TexData );
            if ( GUCEF_NULL != reference._TexData )
            {
                const unsigned char* texel = reference._TexData->Pixels;
                ASSERT_TRUE( ImTextureFormat_RGBA32 == reference._TexData->Format );
                ASSERT_TRUE( 100 == texel[ 0 ] && 150 == texel[ 1 ] && 200 == texel[ 2 ] && 255 == texel[ 3 ] );
            }
            ASSERT_TRUE( GUCEF_NULL == context->GetImageTextureRef( "NoSuchImage" )._TexData );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures: a bitmap font atlas is one texture per font, a font registered again under the name gets a new one" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            CImGuiTextureCache& cache = context->GetTextureCache();
            context->RegisterFont( "Block", MakeTestFont( 6, 9 ) );
            GUI::CWidget* label = context->CreateWidget( "Label", "Label", GUCEF_NULL );
            label->SetProperty( "Size", "0,200 0,20" );
            label->SetProperty( "Font", "Block" );
            label->SetProperty( "Text", "Abc" );
            context->ShowRoot( label, false );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 1 == cache.GetFontTextureCount() );
            ImTextureData* first = cache.GetFontTexture( "Block" );
            ASSERT_TRUE( GUCEF_NULL != first );

            context->RegisterFont( "Block", MakeTestFont( 7, 10 ) );
            context->RenderFrame( 16 );
            ImTextureData* second = cache.GetFontTexture( "Block" );
            ASSERT_TRUE( GUCEF_NULL != second && first != second );
            ASSERT_TRUE( 1 == cache.GetFontTextureCount() );
            ASSERT_TRUE( 7 * 95 == second->Width );
        }
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

}; /* namespace IMGUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
