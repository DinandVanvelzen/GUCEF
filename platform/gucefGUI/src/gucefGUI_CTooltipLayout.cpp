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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CTOOLTIPSERVICE_H
#include "gucefGUI_CTooltipService.h"
#define GUCEF_GUI_CTOOLTIPSERVICE_H
#endif /* GUCEF_GUI_CTOOLTIPSERVICE_H ? */

#include "gucefGUI_CTooltipLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The width of the text in the font, -1 when the font cannot be measured */
Int32
TextWidth( CGUIContext& context, const CString& font, const CString& text )
{
    Int32 width = 0, height = 0;
    if ( !context.MeasureText( font, text, 0, width, height ) )
        return -1;
    return width;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTooltipLayout::CTooltipLayout( void )
    : box()
    , lines()
    , lineHeight( 0 )
    , paddingX( (Int32) CTooltipService::DefaultPaddingX )
    , paddingY( (Int32) CTooltipService::DefaultPaddingY )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTooltipLayout::Wrap( CGUIContext& context, const CString& font, const CString& text, Int32 maxTextWidth, TStringVector& lines )
{GUCEF_TRACE;

    const CString::StringVector paragraphs = text.ParseElements( '\n', true );
    for ( UInt32 p=0; p<(UInt32)paragraphs.size(); ++p )
    {
        const CString& paragraph = paragraphs[ p ];
        const Int32 paragraphWidth = TextWidth( context, font, paragraph );
        if ( maxTextWidth <= 0 || paragraphWidth < 0 || paragraphWidth <= maxTextWidth )
        {
            lines.push_back( paragraph );
            continue;
        }

        const CString::StringVector words = paragraph.ParseElements( ' ', false );
        CString line;
        for ( UInt32 w=0; w<(UInt32)words.size(); ++w )
        {
            const CString& word = words[ w ];
            const CString candidate = line.IsNULLOrEmpty() ? word : line + " " + word;
            if ( TextWidth( context, font, candidate ) <= maxTextWidth )
            {
                line = candidate;
                continue;
            }
            if ( !line.IsNULLOrEmpty() )
            {
                lines.push_back( line );
                line.Clear();
            }
            if ( TextWidth( context, font, word ) <= maxTextWidth )
            {
                line = word;
                continue;
            }
            // a word wider than the room: split between code points
            CString piece;
            const UInt32 length = word.Length();
            for ( UInt32 c=0; c<length; ++c )
            {
                const CString character = word.SubstrFromRange( c, c + 1 );
                const CString next = piece + character;
                if ( !piece.IsNULLOrEmpty() && TextWidth( context, font, next ) > maxTextWidth )
                {
                    lines.push_back( piece );
                    piece = character;
                }
                else
                    piece = next;
            }
            line = piece;
        }
        if ( !line.IsNULLOrEmpty() )
            lines.push_back( line );
    }
}

/*-------------------------------------------------------------------------*/

CRect
CTooltipLayout::Place( Int32 pointerX, Int32 pointerY, Int32 width, Int32 height, Int32 areaWidth, Int32 areaHeight, Int32 offsetX, Int32 offsetY )
{GUCEF_TRACE;

    Int32 x = pointerX + offsetX;
    Int32 y = pointerY + offsetY;
    if ( x + width > areaWidth )
        x = areaWidth - width;
    if ( x < 0 )
        x = 0;
    if ( y + height > areaHeight )
    {
        y = pointerY - height - 4;
        if ( y < 0 )
            y = areaHeight - height;
        if ( y < 0 )
            y = 0;
    }
    return CRect( x, y, width, height );
}

/*-------------------------------------------------------------------------*/

bool
CTooltipLayout::Compute( CGUIContext& context, const CString& font, const CString& text, Int32 pointerX, Int32 pointerY, const CTooltipService& settings )
{GUCEF_TRACE;

    lines.clear();
    box = CRect();
    if ( text.IsNULLOrEmpty() )
        return false;

    paddingX = (Int32) CTooltipService::DefaultPaddingX;
    paddingY = (Int32) CTooltipService::DefaultPaddingY;
    const Int32 areaWidth = (Int32) context.GetLogicalWidth();
    const Int32 areaHeight = (Int32) context.GetLogicalHeight();
    Int32 boxMax = settings.GetMaxWidth();
    if ( areaWidth > 0 && boxMax > areaWidth )
        boxMax = areaWidth;
    Wrap( context, font, text, boxMax - 2 * paddingX, lines );
    if ( lines.empty() )
        return false;

    lineHeight = context.GetLineHeight( font );
    Int32 textWidth = 0;
    for ( UInt32 i=0; i<(UInt32)lines.size(); ++i )
    {
        const Int32 width = TextWidth( context, font, lines[ i ] );
        if ( width > textWidth )
            textWidth = width;
    }
    const Int32 width = textWidth + 2 * paddingX;
    const Int32 height = (Int32) lines.size() * lineHeight + 2 * paddingY;
    box = Place( pointerX, pointerY, width, height, areaWidth, areaHeight, (Int32) CTooltipService::DefaultOffsetX, (Int32) CTooltipService::DefaultOffsetY );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
