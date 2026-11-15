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

/** What a frame builder measured of a font at a size */
struct SFontProbe
{
    ImFont* font;
    Float32 size;
    bool measured;
    Float32 density;
    Float32 lineHeight;             /**< of the rasterized bitmap font, in logical pixels */
    Float32 advanceA;               /**< of Dear ImGui's glyph */
    Float32 expectedAdvanceA;       /**< of the bitmap font, in logical pixels */
    Float32 ascent;
    Float32 baseline;
    bool fallbackIsQuestionMark;
    bool visibleA;
    Float32 glyphWidthB;
    UInt32 differentPixels;         /**< pixels of the glyph B in the atlas of Dear ImGui that differ from its bitmap font */
    GUI::TBitmapFontPtr source;     /**< a bitmap font source to compare the pixels with */

    SFontProbe( void )
        : font( GUCEF_NULL ), size( 16.0f ), measured( false ), density( 0.0f ), lineHeight( 0.0f ), advanceA( 0.0f ), expectedAdvanceA( -1.0f ),
          ascent( 0.0f ), baseline( 0.0f ), fallbackIsQuestionMark( false ), visibleA( false ), glyphWidthB( 0.0f ), differentPixels( 0 ), source()
    {
    }
};

void
ProbeFont( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SFontProbe* probe = static_cast< SFontProbe* >( userData );
    ImGui::PushFont( probe->font, probe->size );
    ImFontBaked* baked = ImGui::GetFontBaked();
    probe->density = baked->RasterizerDensity;
    GUI::TBitmapFontPtr bitmap = CImGuiFontLoader::GetBakedFont( *baked, 0 );
    if ( !bitmap.IsNULL() )
    {
        const bool isBitmapSource = !probe->source.IsNULL();
        const Float32 metricScale = isBitmapSource ? probe->size / (Float32) bitmap->GetLineHeight() : 1.0f / baked->RasterizerDensity;
        probe->lineHeight = bitmap->GetLineHeight() * metricScale;
        probe->baseline = bitmap->GetBaseline() * metricScale;
        const GUI::CBitmapFont::TGlyph* glyph = bitmap->GetGlyph( 'A' );
        if ( GUCEF_NULL != glyph )
            probe->expectedAdvanceA = ( glyph->advance + bitmap->GetSpacing() ) * metricScale;
    }
    probe->ascent = baked->Ascent;
    ImFontGlyph* a = baked->FindGlyph( 'A' );
    if ( GUCEF_NULL != a )
    {
        probe->advanceA = a->AdvanceX;
        probe->visibleA = 0 != a->Visible;
    }
    ImFontGlyph* missing = baked->FindGlyph( (ImWchar) 0x4E00 );
    ImFontGlyph* question = baked->FindGlyph( '?' );
    probe->fallbackIsQuestionMark = GUCEF_NULL != missing && missing == question;

    // the coverage of B in the atlas of Dear ImGui against the atlas of the bitmap font
    ImFontGlyph* b = baked->FindGlyph( 'B' );
    if ( GUCEF_NULL != b )
        probe->glyphWidthB = b->X1 - b->X0;
    if ( !probe->source.IsNULL() && GUCEF_NULL != b && b->Visible )
    {
        ImTextureData* atlas = ImGui::GetIO().Fonts->TexData;
        const GUI::CBitmapFont::TGlyph* glyph = probe->source->GetGlyph( 'B' );
        const UInt8* sourcePixels = static_cast< const UInt8* >( probe->source->GetAtlas()->GetDataPtr() );
        const UInt32 sourceWidth = probe->source->GetAtlas()->GetWidthInPixels();
        const Int32 left = (Int32) ( b->U0 * atlas->Width + 0.5f );
        const Int32 top = (Int32) ( b->V0 * atlas->Height + 0.5f );
        probe->differentPixels = 0;
        for ( Int32 y=0; y<glyph->source.height; ++y )
        {
            for ( Int32 x=0; x<glyph->source.width; ++x )
            {
                const UInt8 expected = sourcePixels[ ( (size_t) ( glyph->source.top + y ) * sourceWidth + glyph->source.left + x ) * 4 + 3 ];
                const unsigned char* texel = static_cast< const unsigned char* >( atlas->GetPixelsAt( left + x, top + y ) );
                const UInt8 actual = ImTextureFormat_RGBA32 == atlas->Format ? texel[ 3 ] : texel[ 0 ];
                if ( expected != actual )
                    ++probe->differentPixels;
            }
        }
    }
    ImGui::PopFont();
    probe->measured = true;
}

/** A bitmap font whose glyphs have a pattern of coverage */
GUI::TBitmapFontPtr
MakePatternFont( Int32 width, Int32 height )
{
    GUI::TBitmapFontPtr font = MakeTestFont( width, height );
    IMAGE::TPixelMapPtr atlas = font->GetAtlas();
    UInt8* data = static_cast< UInt8* >( atlas->GetDataPtr() );
    const UInt32 pixels = atlas->GetWidthInPixels() * atlas->GetHeightInPixels();
    for ( UInt32 i=0; i<pixels; ++i )
        data[ i*4+3 ] = (UInt8) ( ( i * 37 ) % 256 );
    return font;
}

}

/*-------------------------------------------------------------------------*/

void
PerformImGuiFontTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Fonts" );

    GUCEF_TESTFW_TESTCASE( "Fonts: without a font rasterizer an ImGui context is refused, without an assertion" )
    GUCEF_TESTFW_SKIP_IF( !LoadFreeTypeFontPlugin(), "the FreeType font plugin is not there" );
    try
    {
        GUI::CFontRasterizerRegistry& registry = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry();
        GUI::CFontRasterizer* rasterizer = registry.Get( "FreeType" );
        const UInt32 assertions = CImGuiGlobal::Instance()->GetAssertionCount();
        registry.Unregister( "FreeType" );
        GUI::TGuiContextPtr refused = CreateImGuiContext();
        registry.Register( rasterizer );
        ASSERT_TRUE( refused.IsNULL() );
        ASSERT_TRUE( assertions == CImGuiGlobal::Instance()->GetAssertionCount() );
        GUI::TGuiContextPtr accepted = CreateImGuiContext();
        ASSERT_FALSE( accepted.IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fonts: Noto Sans at 13, 16 and 24 pixels and at density 2 has the requested line height and the advances of the bitmap font" )
    GUCEF_TESTFW_SKIP_IF( !LoadFreeTypeFontPlugin(), "the FreeType font plugin is not there" );
    try
    {
        const Float32 sizes[ 3 ] = { 13.0f, 16.0f, 24.0f };
        for ( UInt32 density=1; density<=2; ++density )
        {
            GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
            CImGuiContext* context = AsImGui( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                continue;
            context->SetRenderScale( density );
            ImFont* font = context->GetImGuiContext()->IO.Fonts->Fonts[ 0 ];
            ASSERT_TRUE( CString( font->Sources[ 0 ]->Name ).HasSubstr( "NotoSans" ) >= 0 );
            ASSERT_TRUE( CImGuiFontLoader::GetEmPerLineHeight( *font->Sources[ 0 ] ) > 0.5f );
            for ( UInt32 s=0; s<3; ++s )
            {
                SFontProbe probe;
                probe.font = font;
                probe.size = sizes[ s ];
                CTestFrameBuilder builder( &ProbeFont, &probe );
                builder.Attach( contextPtr );
                context->RenderFrame( 16 );
                ASSERT_TRUE( probe.measured );
                ASSERT_TRUE( (Float32) density == probe.density );
                ASSERT_TRUE( probe.lineHeight >= sizes[ s ] - 1.0f && probe.lineHeight <= sizes[ s ] + 1.0f );
                ASSERT_TRUE( probe.expectedAdvanceA > 0.0f && probe.advanceA == probe.expectedAdvanceA );
                ASSERT_TRUE( probe.ascent == probe.baseline );
                ASSERT_TRUE( probe.visibleA );
                ASSERT_TRUE( probe.fallbackIsQuestionMark );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fonts: the embedded ProggyClean font of Dear ImGui is rasterized by the font loader" )
    GUCEF_TESTFW_SKIP_IF( !LoadFreeTypeFontPlugin(), "the FreeType font plugin is not there" );
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            const UInt32 rasterized = context->GetFontLoader().GetRasterizeCount();
            ImFont* proggy = context->AddDefaultFont();
            ASSERT_TRUE( GUCEF_NULL != proggy );
            ASSERT_TRUE( context->GetFontLoader().GetRasterizeCount() > rasterized );
            if ( GUCEF_NULL != proggy )
            {
                SFontProbe probe;
                probe.font = proggy;
                probe.size = 13.0f;
                CTestFrameBuilder builder( &ProbeFont, &probe );
                builder.Attach( contextPtr );
                context->RenderFrame( 16 );
                ASSERT_TRUE( probe.measured );
                ASSERT_TRUE( probe.visibleA );
                ASSERT_TRUE( probe.advanceA > 0.0f && probe.advanceA == probe.expectedAdvanceA );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fonts: a gucefGUI bitmap font is a source of Dear ImGui with the pixels of its atlas, other sizes scale its quads" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::TBitmapFontPtr pattern = MakePatternFont( 6, 9 );
            ASSERT_TRUE( context->RegisterFont( "Pattern", pattern ) );
            ASSERT_TRUE( GUCEF_NULL == context->AddBitmapFont( "Unknown" ) );
            ImFont* font = context->AddBitmapFont( "Pattern" );
            ASSERT_TRUE( GUCEF_NULL != font );
            if ( GUCEF_NULL != font )
            {
                SFontProbe probe;
                probe.font = font;
                probe.size = 9.0f;
                probe.source = pattern;
                CTestFrameBuilder builder( &ProbeFont, &probe );
                builder.Attach( contextPtr );
                context->RenderFrame( 16 );
                ASSERT_TRUE( probe.measured );
                ASSERT_TRUE( 9.0f == probe.lineHeight );
                ASSERT_TRUE( 6.0f == probe.advanceA );
                ASSERT_TRUE( 6.0f == probe.glyphWidthB );
                ASSERT_TRUE( 0 == probe.differentPixels );

                probe.size = 18.0f;
                probe.measured = false;
                context->RenderFrame( 16 );
                ASSERT_TRUE( probe.measured );
                ASSERT_TRUE( 12.0f == probe.advanceA );
                ASSERT_TRUE( 12.0f == probe.glyphWidthB );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fonts: a font the driver offers is added to the contexts made after it, a font that cannot be read is refused" )
    try
    {
        CTestImGuiDriver& driver = GetTestDriver();
        ASSERT_TRUE( driver.LoadFontFromAsset( GUI::CGuiGlobal::DefaultUiFontPath ) );
        ASSERT_TRUE( 1 == driver.GetAvailableFonts().size() );
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( 2 == context->GetImGuiContext()->IO.Fonts->Fonts.Size );
            ASSERT_TRUE( GUCEF_NULL == context->AddFont( "/no/such/font.ttf", 16.0f ) );
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
