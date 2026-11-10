/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CBITMAPFONT_H
#define GUCEF_GUI_CBITMAPFONT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A font made of pictures: an atlas (RGBA, glyphs in white with their coverage in alpha so a driver
 *  can colour them) plus where each glyph sits in it. An application registers it with a GUI
 *  context under a name; widgets then use that name as their font on any backend.
 *
 *  The text measuring and line breaking here are the rules every driver follows for bitmap fonts,
 *  so text looks the same on every backend.
 */
class GUCEF_GUI_PUBLIC_CPP CBitmapFont
{
    public:

    struct SGlyph
    {
        CRect source;       /**< where the glyph is in the atlas */
        Int32 offsetX;      /**< where the top left of the glyph goes relative to the pen position */
        Int32 offsetY;      /**< relative to the top of the line */
        Int32 advance;      /**< how far the pen moves after the glyph */

        SGlyph( void );
    };
    typedef struct SGlyph TGlyph;

    typedef GUCEF::map< UInt32, TGlyph > TGlyphMap;
    typedef GUCEF::vector< CString > TStringVector;

    CBitmapFont( void );
    ~CBitmapFont();

    void SetAtlas( const IMAGE::TPixelMapPtr& atlas );
    const IMAGE::TPixelMapPtr& GetAtlas( void ) const;

    void SetGlyph( UInt32 codePoint, const TGlyph& glyph );
    const TGlyph* GetGlyph( UInt32 codePoint ) const;
    const TGlyphMap& GetGlyphs( void ) const;

    /** The glyph used for code points the font does not have, 0 for none */
    void SetFallbackCodePoint( UInt32 codePoint );
    UInt32 GetFallbackCodePoint( void ) const;

    void SetLineHeight( Int32 lineHeight );
    Int32 GetLineHeight( void ) const;

    /** Extra pixels between two glyphs, on top of their advance */
    void SetSpacing( Int32 spacing );
    Int32 GetSpacing( void ) const;

    /** Distance from the top of the line to the baseline */
    void SetBaseline( Int32 baseline );
    Int32 GetBaseline( void ) const;

    /**
     *  Atlas pixels per logical pixel in each direction, 1 by default. The source rectangles of the glyphs
     *  are in atlas pixels, everything else (offsets, advance, line height, spacing, baseline) in logical
     *  pixels: a glyph covers its source size divided by the density. A font for a render scale (see
     *  CGUIContext::SetRenderScale()) has its atlas at that density.
     */
    void SetDensity( UInt32 density );
    UInt32 GetDensity( void ) const;

    /** The glyph for the code point, the fallback glyph when the font lacks it, NULL when neither exists */
    const TGlyph* ResolveGlyph( UInt32 codePoint ) const;

    /** Width in pixels of one line of text (no line breaks are interpreted) */
    Int32 GetTextWidth( const CString& text ) const;

    /**
     *  Breaks the text into lines: on '\n' always, and when 'maxWidth' > 0 also between words so no
     *  line is wider than 'maxWidth' (a single word wider than that is broken between characters)
     */
    TStringVector BreakLines( const CString& text, Int32 maxWidth ) const;

    /** The size of the text when broken with BreakLines() */
    void MeasureText( const CString& text, Int32 maxWidth, Int32& width, Int32& height ) const;

    private:

    IMAGE::TPixelMapPtr m_atlas;
    TGlyphMap m_glyphs;
    UInt32 m_fallback;
    Int32 m_lineHeight;
    Int32 m_spacing;
    Int32 m_baseline;
    UInt32 m_density;
};

typedef CORE::CTSharedPtr< CBitmapFont, MT::CMutex > TBitmapFontPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CBITMAPFONT_H ? */
