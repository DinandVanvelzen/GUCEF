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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#include "gucefGUI_CBitmapFont.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBitmapFont::SGlyph::SGlyph( void )
    : source()
    , offsetX( 0 )
    , offsetY( 0 )
    , advance( 0 )
{
}

/*-------------------------------------------------------------------------*/

CBitmapFont::CBitmapFont( void )
    : m_atlas()
    , m_glyphs()
    , m_fallback( 0 )
    , m_lineHeight( 0 )
    , m_spacing( 0 )
    , m_baseline( 0 )
    , m_density( 1 )
{
}

/*-------------------------------------------------------------------------*/

CBitmapFont::~CBitmapFont()
{
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetAtlas( const IMAGE::TPixelMapPtr& atlas )
{
    m_atlas = atlas;
}

/*-------------------------------------------------------------------------*/

const IMAGE::TPixelMapPtr&
CBitmapFont::GetAtlas( void ) const
{
    return m_atlas;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetGlyph( UInt32 codePoint, const TGlyph& glyph )
{
    m_glyphs[ codePoint ] = glyph;
}

/*-------------------------------------------------------------------------*/

const CBitmapFont::TGlyph*
CBitmapFont::GetGlyph( UInt32 codePoint ) const
{
    TGlyphMap::const_iterator i = m_glyphs.find( codePoint );
    if ( i != m_glyphs.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CBitmapFont::TGlyphMap&
CBitmapFont::GetGlyphs( void ) const
{
    return m_glyphs;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetFallbackCodePoint( UInt32 codePoint )
{
    m_fallback = codePoint;
}

/*-------------------------------------------------------------------------*/

UInt32
CBitmapFont::GetFallbackCodePoint( void ) const
{
    return m_fallback;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetLineHeight( Int32 lineHeight )
{
    m_lineHeight = lineHeight;
}

/*-------------------------------------------------------------------------*/

Int32
CBitmapFont::GetLineHeight( void ) const
{
    return m_lineHeight;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetSpacing( Int32 spacing )
{
    m_spacing = spacing;
}

/*-------------------------------------------------------------------------*/

Int32
CBitmapFont::GetSpacing( void ) const
{
    return m_spacing;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetBaseline( Int32 baseline )
{
    m_baseline = baseline;
}

/*-------------------------------------------------------------------------*/

Int32
CBitmapFont::GetBaseline( void ) const
{
    return m_baseline;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::SetDensity( UInt32 density )
{
    m_density = density > 0 ? density : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CBitmapFont::GetDensity( void ) const
{
    return m_density;
}

/*-------------------------------------------------------------------------*/

const CBitmapFont::TGlyph*
CBitmapFont::ResolveGlyph( UInt32 codePoint ) const
{
    const TGlyph* glyph = GetGlyph( codePoint );
    if ( GUCEF_NULL == glyph && 0 != m_fallback )
        glyph = GetGlyph( m_fallback );
    return glyph;
}

/*-------------------------------------------------------------------------*/

Int32
CBitmapFont::GetTextWidth( const CString& text ) const
{
    TCodePointVector codePoints;
    DecodeUtf8( text, codePoints );

    Int32 width = 0;
    for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
    {
        const TGlyph* glyph = ResolveGlyph( codePoints[ i ] );
        if ( GUCEF_NULL == glyph )
            continue;
        if ( width > 0 )
            width += m_spacing;
        width += glyph->advance;
    }
    return width;
}

/*-------------------------------------------------------------------------*/

CBitmapFont::TStringVector
CBitmapFont::BreakLines( const CString& text, Int32 maxWidth ) const
{
    TStringVector lines;
    CString::StringVector paragraphs = text.ParseElements( '\n', true );
    for ( UInt32 p=0; p<(UInt32)paragraphs.size(); ++p )
    {
        const CString& paragraph = paragraphs[ p ];
        if ( maxWidth <= 0 || GetTextWidth( paragraph ) <= maxWidth )
        {
            lines.push_back( paragraph );
            continue;
        }

        // greedy: as many words as fit, a word wider than the line is split between characters
        CString::StringVector words = paragraph.ParseElements( ' ', true );
        CString line;
        for ( UInt32 w=0; w<(UInt32)words.size(); ++w )
        {
            CString candidate = line.IsNULLOrEmpty() ? words[ w ] : line + ' ' + words[ w ];
            if ( GetTextWidth( candidate ) <= maxWidth )
            {
                line = candidate;
                continue;
            }
            if ( !line.IsNULLOrEmpty() )
            {
                lines.push_back( line );
                line.Clear();
            }

            CString word = words[ w ];
            while ( GetTextWidth( word ) > maxWidth )
            {
                TCodePointVector codePoints;
                DecodeUtf8( word, codePoints );
                CString part;
                UInt32 used = 0;
                for ( ; used<(UInt32)codePoints.size(); ++used )
                {
                    CString next = part;
                    next.Append( codePoints[ used ] );
                    if ( used > 0 && GetTextWidth( next ) > maxWidth )
                        break;
                    part = next;
                }
                lines.push_back( part );
                CString rest;
                for ( UInt32 r=used; r<(UInt32)codePoints.size(); ++r )
                    rest.Append( codePoints[ r ] );
                word = rest;
                if ( word.IsNULLOrEmpty() )
                    break;
            }
            line = word;
        }
        lines.push_back( line );
    }
    return lines;
}

/*-------------------------------------------------------------------------*/

void
CBitmapFont::MeasureText( const CString& text, Int32 maxWidth, Int32& width, Int32& height ) const
{
    TStringVector lines = BreakLines( text, maxWidth );
    width = 0;
    for ( UInt32 i=0; i<(UInt32)lines.size(); ++i )
    {
        Int32 lineWidth = GetTextWidth( lines[ i ] );
        if ( lineWidth > width )
            width = lineWidth;
    }
    height = (Int32) lines.size() * m_lineHeight;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
