/*
 *  fontpluginFREETYPE: GUCEF GUI plugin that rasterizes TrueType and OpenType fonts with FreeType
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

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "fontpluginFREETYPE_CFreeTypeFontRasterizer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace FONTPLUGINFREETYPE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** One rendered glyph waiting for its place in the atlas */
struct SRenderedGlyph
{
    UInt32 codePoint;
    Int32 width;
    Int32 height;
    Int32 left;
    Int32 top;
    Int32 advance;
    Int32 atlasX;
    Int32 atlasY;
    GUCEF::vector< UInt8 > coverage;
};
typedef GUCEF::vector< SRenderedGlyph > TRenderedGlyphs;

const Int32 GlyphPadding = 1;

/** 26.6 fixed point to whole pixels, rounded up */
Int32
CeilPixels( FT_Pos value )
{
    return (Int32) ( ( value + 63 ) >> 6 );
}

/*-------------------------------------------------------------------------*/

/** Places the glyphs in rows of at most 'atlasWidth' pixels and returns the height the atlas needs */
Int32
PackGlyphs( TRenderedGlyphs& glyphs, Int32 atlasWidth )
{
    Int32 x = GlyphPadding;
    Int32 y = GlyphPadding;
    Int32 rowHeight = 0;
    for ( UInt32 i=0; i<(UInt32)glyphs.size(); ++i )
    {
        SRenderedGlyph& glyph = glyphs[ i ];
        if ( 0 == glyph.width || 0 == glyph.height )
            continue;
        if ( x + glyph.width + GlyphPadding > atlasWidth )
        {
            x = GlyphPadding;
            y += rowHeight + GlyphPadding;
            rowHeight = 0;
        }
        glyph.atlasX = x;
        glyph.atlasY = y;
        x += glyph.width + GlyphPadding;
        if ( glyph.height > rowHeight )
            rowHeight = glyph.height;
    }
    return y + rowHeight + GlyphPadding;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFreeTypeFontRasterizer::CFreeTypeFontRasterizer( void )
    : GUI::CFontRasterizer()
    , m_library( GUCEF_NULL )
{GUCEF_TRACE;

    FT_Library library = GUCEF_NULL;
    FT_Error error = FT_Init_FreeType( &library );
    if ( 0 != error )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FreeTypeFontRasterizer: FreeType did not start, error " << (Int32) error );
        return;
    }
    m_library = library;
}

/*-------------------------------------------------------------------------*/

CFreeTypeFontRasterizer::~CFreeTypeFontRasterizer()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_library )
        FT_Done_FreeType( static_cast< FT_Library >( m_library ) );
    m_library = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFreeTypeFontRasterizer::IsInitialized( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_library;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFreeTypeFontRasterizer::GetName( void ) const
{GUCEF_TRACE;

    return "FreeType";
}

/*-------------------------------------------------------------------------*/

bool
CFreeTypeFontRasterizer::CanRasterize( const CORE::CDynamicBuffer& fontData ) const
{GUCEF_TRACE;

    if ( fontData.GetDataSize() < 4 )
        return false;

    // TrueType, OpenType with CFF outlines, Apple TrueType, font collections
    const UInt8* tag = static_cast< const UInt8* >( fontData.GetConstBufferPtr() );
    return ( 0x00 == tag[ 0 ] && 0x01 == tag[ 1 ] && 0x00 == tag[ 2 ] && 0x00 == tag[ 3 ] ) ||
           0 == memcmp( tag, "OTTO", 4 ) ||
           0 == memcmp( tag, "true", 4 ) ||
           0 == memcmp( tag, "ttcf", 4 );
}

/*-------------------------------------------------------------------------*/

GUI::TBitmapFontPtr
CFreeTypeFontRasterizer::Rasterize( const CORE::CDynamicBuffer& fontData ,
                                    UInt32 pixelSize                     ,
                                    const TCodePointRanges& ranges       )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_library || 0 == pixelSize || !CanRasterize( fontData ) )
        return GUI::TBitmapFontPtr();

    FT_Face face = GUCEF_NULL;
    FT_Error error = FT_New_Memory_Face( static_cast< FT_Library >( m_library )                          ,
                                         static_cast< const FT_Byte* >( fontData.GetConstBufferPtr() ) ,
                                         (FT_Long) fontData.GetDataSize()                              ,
                                         0                                                             ,
                                         &face                                                         );
    if ( 0 != error )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FreeTypeFontRasterizer: Cannot read the font, error " << (Int32) error );
        return GUI::TBitmapFontPtr();
    }
    if ( 0 != FT_Set_Pixel_Sizes( face, 0, pixelSize ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FreeTypeFontRasterizer: The font has no size " << pixelSize );
        FT_Done_Face( face );
        return GUI::TBitmapFontPtr();
    }

    // render every glyph the font has in the ranges
    TRenderedGlyphs glyphs;
    Int64 area = 0;
    for ( UInt32 r=0; r<(UInt32)ranges.size(); ++r )
    {
        for ( UInt32 codePoint=ranges[ r ].first; codePoint<=ranges[ r ].last; ++codePoint )
        {
            FT_UInt glyphIndex = FT_Get_Char_Index( face, codePoint );
            if ( 0 == glyphIndex && ' ' != codePoint )
                continue;
            if ( 0 != FT_Load_Glyph( face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL ) )
                continue;

            FT_GlyphSlot slot = face->glyph;
            const FT_Bitmap& bitmap = slot->bitmap;
            if ( 0 != bitmap.rows && FT_PIXEL_MODE_GRAY != bitmap.pixel_mode )
                continue;

            SRenderedGlyph glyph;
            glyph.codePoint = codePoint;
            glyph.width = (Int32) bitmap.width;
            glyph.height = (Int32) bitmap.rows;
            glyph.left = (Int32) slot->bitmap_left;
            glyph.top = (Int32) slot->bitmap_top;
            glyph.advance = (Int32) ( ( slot->advance.x + 32 ) >> 6 );
            glyph.atlasX = 0;
            glyph.atlasY = 0;
            glyph.coverage.resize( (size_t) ( glyph.width * glyph.height ) );
            for ( Int32 y=0; y<glyph.height; ++y )
            {
                const UInt8* row = bitmap.buffer + ( bitmap.pitch >= 0 ? y : ( y - glyph.height + 1 ) ) * bitmap.pitch;
                for ( Int32 x=0; x<glyph.width; ++x )
                    glyph.coverage[ (size_t) ( y * glyph.width + x ) ] = row[ x ];
            }
            area += (Int64) ( glyph.width + GlyphPadding ) * ( glyph.height + GlyphPadding );
            glyphs.push_back( glyph );

            if ( codePoint == ranges[ r ].last )
                break;  // the last code point of a range can be the largest value an UInt32 holds
        }
    }

    Int32 ascender = CeilPixels( face->size->metrics.ascender );
    Int32 lineHeight = CeilPixels( face->size->metrics.height );
    FT_Done_Face( face );

    if ( glyphs.empty() )
        return GUI::TBitmapFontPtr();

    // a square-ish atlas: the smallest power of two width that holds the glyphs in about as many rows
    Int32 atlasWidth = 64;
    while ( (Int64) atlasWidth * atlasWidth < area * 2 && atlasWidth < 4096 )
        atlasWidth *= 2;
    Int32 atlasHeight = PackGlyphs( glyphs, atlasWidth );

    IMAGE::TPixelMapPtr atlas( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, (UInt32) atlasWidth, (UInt32) atlasHeight, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* pixels = static_cast< UInt8* >( atlas->GetDataPtr() );
    for ( Int32 i=0; i<atlasWidth*atlasHeight; ++i )
    {
        pixels[ i*4 ] = 255;
        pixels[ i*4+1 ] = 255;
        pixels[ i*4+2 ] = 255;
        pixels[ i*4+3 ] = 0;
    }

    GUI::TBitmapFontPtr font( GUCEF_NEW GUI::CBitmapFont() );
    for ( UInt32 i=0; i<(UInt32)glyphs.size(); ++i )
    {
        const SRenderedGlyph& rendered = glyphs[ i ];
        for ( Int32 y=0; y<rendered.height; ++y )
        {
            UInt8* row = pixels + ( (size_t) ( rendered.atlasY + y ) * atlasWidth + rendered.atlasX ) * 4;
            for ( Int32 x=0; x<rendered.width; ++x )
                row[ x*4+3 ] = rendered.coverage[ (size_t) ( y * rendered.width + x ) ];
        }

        GUI::CBitmapFont::TGlyph glyph;
        glyph.source = GUI::CRect( rendered.atlasX, rendered.atlasY, rendered.width, rendered.height );
        glyph.offsetX = rendered.left;
        glyph.offsetY = ascender - rendered.top;
        glyph.advance = rendered.advance;
        font->SetGlyph( rendered.codePoint, glyph );
    }

    font->SetAtlas( atlas );
    font->SetLineHeight( lineHeight );
    font->SetBaseline( ascender );
    font->SetSpacing( 0 );
    if ( GUCEF_NULL != font->GetGlyph( '?' ) )
        font->SetFallbackCodePoint( '?' );
    return font;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FONTPLUGINFREETYPE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
