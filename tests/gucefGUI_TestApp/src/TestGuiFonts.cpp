/*
 *  gucefGUI_TestApp: GUCEF test application for the GUI module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** The alpha of an atlas pixel */
UInt8
AlphaAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return data[ 3 ];
}

/** The bounds of the pixels in a rectangle that are not the background colour, empty when there are none */
GUI::CRect
InkBounds( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect, UInt8 background )
{
    Int32 left = rect.Right(), top = rect.Bottom(), right = rect.left, bottom = rect.top;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
    {
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
        {
            const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
            if ( data[ 0 ] == background && data[ 1 ] == background && data[ 2 ] == background )
                continue;
            if ( x < left ) left = x;
            if ( y < top ) top = y;
            if ( x + 1 > right ) right = x + 1;
            if ( y + 1 > bottom ) bottom = y + 1;
        }
    }
    if ( right <= left || bottom <= top )
        return GUI::CRect();
    return GUI::CRect( left, top, right - left, bottom - top );
}

}

/*-------------------------------------------------------------------------*/

void
PerformGuiFontTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Fonts" );

    GUCEF_TESTFW_TESTCASE( "Fonts: Without a rasterizer that reads it, font data gives no bitmap font" )
    try
    {
        GUI::CFontRasterizerRegistry registry;
        CORE::CDynamicBuffer notAFont;
        notAFont.CopyFrom( 8, "not font" );
        ASSERT_FALSE( registry.HasRasterizers() );
        ASSERT_TRUE( registry.RasterizeFont( notAFont, 16, GUI::CFontRasterizer::GetLatin1Ranges() ).IsNULL() );

        if ( LoadFreeTypeFontPlugin() )
        {
            GUI::CFontRasterizer* freeType = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().Get( "FreeType" );
            ASSERT_FALSE( freeType->CanRasterize( notAFont ) );
            ASSERT_TRUE( freeType->Rasterize( notAFont, 16, GUI::CFontRasterizer::GetLatin1Ranges() ).IsNULL() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fonts: The FreeType plugin rasterizes the user interface font gucefGUI comes with" )
    if ( !LoadFreeTypeFontPlugin() )
        GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );
    try
    {
        ASSERT_TRUE( GUI::CGuiGlobal::Instance()->MountData() );
        ASSERT_TRUE( GUI::CGuiGlobal::Instance()->MountData() );

        GUI::CFontRasterizerRegistry& registry = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry();
        ASSERT_TRUE( registry.HasRasterizers() && 1 == registry.GetNames().count( "FreeType" ) );
        GUI::TBitmapFontPtr font = registry.RasterizeFont( GUI::CGuiGlobal::DefaultUiFontPath, 20, GUI::CFontRasterizer::GetLatin1Ranges() );
        ASSERT_TRUE( !font.IsNULL() && !font->GetAtlas().IsNULL() );
        if ( !font.IsNULL() && !font->GetAtlas().IsNULL() && GUCEF_NULL != font->GetGlyph( 'H' ) )
        {
            // every code point of the ranges is there: 95 ASCII and 96 Latin-1
            ASSERT_TRUE( 95 + 96 == font->GetGlyphs().size() );
            ASSERT_TRUE( font->GetLineHeight() >= 20 && font->GetLineHeight() <= 40 );
            ASSERT_TRUE( font->GetBaseline() > 10 && font->GetBaseline() < font->GetLineHeight() );

            // the space moves the pen and draws nothing, a W is wider than an i
            const GUI::CBitmapFont::TGlyph* space = font->GetGlyph( ' ' );
            ASSERT_TRUE( GUCEF_NULL != space && space->advance > 0 && space->source.IsEmpty() );
            ASSERT_TRUE( font->GetGlyph( 'W' )->advance > font->GetGlyph( 'i' )->advance );

            // an H stands on the baseline and its coverage is anti-aliased: solid in the stems, partial at some edges
            const GUI::CBitmapFont::TGlyph* h = font->GetGlyph( 'H' );
            ASSERT_TRUE( GUCEF_NULL != h && h->offsetY + h->source.height == font->GetBaseline() );
            const IMAGE::TPixelMapPtr& atlas = font->GetAtlas();
            UInt32 solid = 0, partial = 0;
            for ( Int32 y=h->source.top; y<h->source.Bottom(); ++y )
            {
                for ( Int32 x=h->source.left; x<h->source.Right(); ++x )
                {
                    UInt8 alpha = AlphaAt( atlas, x, y );
                    if ( 255 == alpha )
                        ++solid;
                    else if ( 0 < alpha )
                        ++partial;
                }
            }
            ASSERT_TRUE( solid > 20 && partial > 0 );

            // the glyphs of a font do not overlap in the atlas
            const GUI::CBitmapFont::TGlyph* e = font->GetGlyph( 0xE9 );
            ASSERT_TRUE( GUCEF_NULL != e && !e->source.IsEmpty() );
            GUI::CRect overlap = h->source.Intersection( e->source );
            ASSERT_TRUE( overlap.IsEmpty() );

            // code points it does not have become a question mark
            ASSERT_TRUE( '?' == font->GetFallbackCodePoint() );
            ASSERT_TRUE( font->ResolveGlyph( 0x4E2D ) == font->GetGlyph( '?' ) );

            // a bigger size gives bigger glyphs
            GUI::TBitmapFontPtr big = registry.RasterizeFont( GUI::CGuiGlobal::DefaultUiFontPath, 40, GUI::CFontRasterizer::GetLatin1Ranges() );
            ASSERT_TRUE( !big.IsNULL() && big->GetGlyph( 'H' )->source.height > h->source.height + 10 );
        }

        ASSERT_TRUE( registry.RasterizeFont( "gucefGUI/fonts/NoSuchFont.ttf", 20, GUI::CFontRasterizer::GetLatin1Ranges() ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fonts: Text in a rasterized font is drawn where its glyph metrics put it" )
    if ( !LoadFreeTypeFontPlugin() )
        GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context && GUI::CGuiGlobal::Instance()->MountData() );

        GUI::TBitmapFontPtr font = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().RasterizeFont( GUI::CGuiGlobal::DefaultUiFontPath, 24, GUI::CFontRasterizer::GetLatin1Ranges() );
        ASSERT_TRUE( GUCEF_NULL != context && !font.IsNULL() && context->RegisterFont( "UI", font ) );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
            screen->SetProperty( "Size", "1,0 1,0" );
            screen->SetColourProperty( "BackgroundColour", GUI::CColour( 255, 255, 255, 255 ) );
            context->ShowRoot( screen, false );
            GUI::CWidget* label = context->CreateWidget( "Label", "Label", screen );
            label->SetProperty( "Position", "0,100 0,50" );
            label->SetProperty( "Size", "0,200 0,60" );
            label->SetProperty( "Font", "UI" );
            label->SetProperty( "Text", "H" );
            label->SetColourProperty( "TextColour", GUI::CColour( 0, 0, 0, 255 ) );

            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( context->CaptureFrame( pixels ) && !pixels.IsNULL() );
            if ( !pixels.IsNULL() && !font.IsNULL() && GUCEF_NULL != font->GetGlyph( 'H' ) )
            {
                // the ink is the H glyph: as wide and as high as its picture, inside the label, nothing outside of it
                const GUI::CBitmapFont::TGlyph* h = font->GetGlyph( 'H' );
                GUI::CRect ink = InkBounds( pixels, GUI::CRect( 0, 0, 640, 480 ), 255 );
                ASSERT_TRUE( ink.width == h->source.width && ink.height == h->source.height );
                ASSERT_TRUE( ink.left >= 100 && ink.top >= 50 && ink.Right() <= 300 && ink.Bottom() <= 110 );

                // the stems are solid text colour
                UInt32 black = 0;
                for ( Int32 y=ink.top; y<ink.Bottom(); ++y )
                {
                    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * 640 + ink.left + 1 ) * 4;
                    if ( data[ 0 ] < 8 && data[ 1 ] < 8 && data[ 2 ] < 8 )
                        ++black;
                }
                ASSERT_TRUE( black + 2 >= (UInt32) ink.height );
            }
            context->DestroyWidget( screen );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
