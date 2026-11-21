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

#include <stdio.h>
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#include "gucefGUI_CButton.h"
#include "gucefGUI_CEditbox.h"
#include "gucefGUI_CSecretInput.h"
#include "gucefCORE_SecureMemory.h"
#include "gucefGUI_CTextbox.h"
#include "gucefGUI_CListBox.h"
#include "gucefGUI_CCombobox.h"
#include "gucefGUI_CSpinner.h"
#include "gucefGUI_CSlider.h"
#include "gucefGUI_CSplitter.h"
#include "gucefGUI_CFoldablePanel.h"
#include "gucefGUI_CProgressBar.h"
#include "gucefGUI_CDrawSurface.h"
#include "gucefGUI_CTabControl.h"
#include "gucefGUI_CTabControlPainter.h"
#include "gucefGUI_CTreeview.h"
#include "gucefGUI_CMenuBar.h"
#include "gucefGUI_CPopupMenu.h"
#include "gucefGUI_CGridView.h"
#include "gucefGUI_CGridViewPainter.h"
#include "gucefGUI_CAudioPlayer.h"
#include "gucefGUI_CWindow.h"
#include "gucefGUI_CScrollPanel.h"
#include "gucefGUI_CTooltipLayout.h"
#include "gucefGUI_CTooltipPainter.h"
#include "gucefGUI_CLayoutDebugOverlay.h"

#include "gucefGUI_CWidgetPainter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The look of widgets that have no picture or colour of their own */
const CColour BoxColour( 48, 48, 52, 255 );
const CColour BoxHoverColour( 70, 70, 76, 255 );
const CColour BoxPressedColour( 32, 32, 36, 255 );
const CColour BoxBorderColour( 120, 120, 128, 255 );
const CColour FieldColour( 20, 20, 24, 255 );
const CColour DisabledTextColour( 128, 128, 128, 255 );
const CColour FocusColour( 200, 200, 90, 255 );
const CColour AccentColour( 74, 136, 232, 255 );          /* the accent of the standard theme: focus, selection, and the rows that need the player */
const CColour AccentTint( 74, 136, 232, 56 );

bool
IsA( const CWidget& widget, const char* type )
{
    const CResolvedWidgetType* resolved = widget.GetResolvedType();
    return GUCEF_NULL != resolved && resolved->IsA( type );
}

/** The widget as the class T when its registered type is T::GetRegisteredTypeName() or derives from it, else NULL; no run time type information needed */
template < class T >
const T*
AsType( const CWidget& widget )
{
    return IsA( widget, T::GetRegisteredTypeName() ) ? static_cast< const T* >( &widget ) : GUCEF_NULL;
}

/** A colour property, 'fallback' when it is not set (empty) or no colour */
CColour
ColourOr( const CWidget& widget, EWidgetProperty property, const CColour& fallback )
{
    CColour colour;
    if ( !widget.TryGetColourProperty( property, colour ) )
        return fallback;
    return colour;
}

void
Fill( CWidgetPainter::TCommandVector& commands, const CRect& rect, const CColour& colour )
{
    if ( rect.IsEmpty() || 0 == colour.a )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_FILLRECT;
    command.rect = rect;
    command.colour = colour;
    commands.push_back( command );
}

void
Stroke( CWidgetPainter::TCommandVector& commands, const CRect& rect, const CColour& colour, Int32 thickness )
{
    if ( rect.IsEmpty() || 0 == colour.a || thickness <= 0 )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_STROKERECT;
    command.rect = rect;
    command.colour = colour;
    command.thickness = thickness;
    commands.push_back( command );
}

void
Image( CWidgetPainter::TCommandVector& commands, const CString& image, const CRect& target, const CRect& source, const CColour& tint )
{
    if ( image.IsNULLOrEmpty() || target.IsEmpty() )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_IMAGE;
    command.image = image;
    command.rect = target;
    command.source = source;
    command.colour = tint;
    commands.push_back( command );
}

void
Line( CWidgetPainter::TCommandVector& commands, Int32 x0, Int32 y0, Int32 x1, Int32 y1, const CColour& colour )
{
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_LINE;
    command.x0 = x0;
    command.y0 = y0;
    command.x1 = x1;
    command.y1 = y1;
    command.colour = colour;
    commands.push_back( command );
}

void
Text( CWidgetPainter::TCommandVector& commands, const CString& font, const CString& text, Int32 x, Int32 y, const CColour& colour,
      UInt32 textFlags = CDrawSurface::TEXTFLAG_NONE, const CRect& textArea = CRect() )
{
    if ( text.IsNULLOrEmpty() || 0 == colour.a )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_TEXT;
    command.font = font;
    command.text = text;
    command.x0 = x;
    command.y0 = y;
    command.colour = colour;
    command.textFlags = textFlags;
    command.textArea = textArea;
    commands.push_back( command );
}

/** The TextOverflow of a widget whose type has the property, 'fallback' when it has none (an older layout, a type without it) */
ETextOverflow
OverflowOf( const CWidget& widget, ETextOverflow fallback )
{
    return StringToTextOverflow( widget.GetPropertyRef( EWidgetProperty_TextOverflow ), fallback );
}

/** Whether the byte starts a code point (it is not a continuation byte of UTF-8) */
bool
StartsCodePoint( const CString& text, UInt32 index )
{
    return index >= text.Length() || 0x80 != ( (UInt32)(unsigned char) text[ index ] & 0xC0 );
}

/** The longest start of the text (in bytes, on a code point boundary) that is at most 'room' pixels wide */
UInt32
LongestStart( CGUIContext& context, const CString& font, const CString& text, Int32 room )
{
    UInt32 low = 0;
    UInt32 high = text.Length();
    Int32 width = 0, height = 0;
    while ( low < high )
    {
        UInt32 middle = ( low + high + 1 ) / 2;
        context.MeasureText( font, text.SubstrFromRange( 0, middle ), 0, width, height );
        if ( width <= room )
            low = middle;
        else
            high = middle - 1;
    }
    while ( low > 0 && !StartsCodePoint( text, low ) )
        --low;
    return low;
}

/** Where the longest end of the text starts (in bytes, on a code point boundary) that is at most 'room' pixels wide */
UInt32
LongestEnd( CGUIContext& context, const CString& font, const CString& text, Int32 room )
{
    UInt32 low = 0;
    UInt32 high = text.Length();
    Int32 width = 0, height = 0;
    while ( low < high )
    {
        UInt32 middle = ( low + high ) / 2;
        context.MeasureText( font, text.SubstrFromRange( middle, text.Length() ), 0, width, height );
        if ( width <= room )
            high = middle;
        else
            low = middle + 1;
    }
    while ( low < text.Length() && !StartsCodePoint( text, low ) )
        ++low;
    return low;
}

void
PushClip( CWidgetPainter::TCommandVector& commands, const CRect& rect )
{
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_PUSHCLIP;
    command.rect = rect;
    commands.push_back( command );
}

void
PopClip( CWidgetPainter::TCommandVector& commands )
{
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_POPCLIP;
    commands.push_back( command );
}

/** Clips what is added to the commands while the scope lives: the clip is popped when the scope ends, on every way out of the block */
class CClipScope
{
    public:

    CClipScope( CWidgetPainter::TCommandVector& commands, const CRect& rect )
        : m_commands( commands )
    {
        PushClip( m_commands, rect );
    }

    ~CClipScope()
    {
        PopClip( m_commands );
    }

    private:

    CClipScope( const CClipScope& src );
    CClipScope& operator=( const CClipScope& src );

    CWidgetPainter::TCommandVector& m_commands;
};

/** The text colour, grey when the widget is disabled */
CColour
TextColourOf( const CWidget& widget )
{
    if ( !widget.IsEffectivelyEnabled() )
        return ColourOr( widget, EWidgetProperty_DisabledTextColour, DisabledTextColour );
    return ColourOr( widget, EWidgetProperty_TextColour, CColour( 255, 255, 255, 255 ) );
}

/** A small arrow pointing down, for the combo box */
void
DownArrow( CWidgetPainter::TCommandVector& commands, const CRect& box, const CColour& colour )
{
    Int32 cx = box.left + box.width / 2;
    Int32 cy = box.top + box.height / 2;
    for ( Int32 i=0; i<4; ++i )
        Line( commands, cx - 3 + i, cy - 1 + i, cx + 3 - i, cy - 1 + i, colour );
}

/** The glyph of the fold button of a window: a chevron up that rolls the window up, down that unrolls it, two pixels thick */
void
FoldChevron( CWidgetPainter::TCommandVector& commands, const CRect& box, bool collapsed, const CColour& colour )
{
    Int32 cx = box.left + box.width / 2;
    Int32 cy = box.top + box.height / 2;
    for ( Int32 thickness=0; thickness<2; ++thickness )
    {
        if ( collapsed )
        {
            Line( commands, cx - 5, cy - 3 + thickness, cx, cy + 2 + thickness, colour );
            Line( commands, cx, cy + 2 + thickness, cx + 5, cy - 3 + thickness, colour );
        }
        else
        {
            Line( commands, cx - 5, cy + 2 + thickness, cx, cy - 3 + thickness, colour );
            Line( commands, cx, cy - 3 + thickness, cx + 5, cy + 2 + thickness, colour );
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRect
CWidgetPainter::FitImage( Int32 imageWidth, Int32 imageHeight, const CRect& area, const CString& scaleMode )
{GUCEF_TRACE;

    if ( imageWidth <= 0 || imageHeight <= 0 )
        return CRect();
    if ( scaleMode == "None" )
        return CRect( area.left, area.top, imageWidth, imageHeight );
    if ( scaleMode == "Center" )
        return CRect( area.left + ( area.width - imageWidth ) / 2, area.top + ( area.height - imageHeight ) / 2, imageWidth, imageHeight );
    if ( scaleMode == "Fit" )
    {
        Int32 w = area.width;
        Int32 h = (Int32) ( (Int64) area.width * imageHeight / imageWidth );
        if ( h > area.height )
        {
            h = area.height;
            w = (Int32) ( (Int64) area.height * imageWidth / imageHeight );
        }
        return CRect( area.left + ( area.width - w ) / 2, area.top + ( area.height - h ) / 2, w, h );
    }
    if ( scaleMode == "Integer" )
    {
        Int32 scale = area.width / imageWidth;
        if ( area.height / imageHeight < scale )
            scale = area.height / imageHeight;
        if ( scale < 1 )
            return FitImage( imageWidth, imageHeight, area, "Fit" );
        Int32 w = imageWidth * scale;
        Int32 h = imageHeight * scale;
        return CRect( area.left + ( area.width - w ) / 2, area.top + ( area.height - h ) / 2, w, h );
    }
    return area;   // Stretch
}

/*-------------------------------------------------------------------------*/

Int32
CWidgetPainter::AlignText( Int32 left, Int32 width, Int32 textWidth, const CString& hAlign, Int32 padLeft, Int32 padRight )
{GUCEF_TRACE;

    if ( "Right" == hAlign )
        return left + width - padRight - textWidth;
    if ( "Center" == hAlign )
        return left + padLeft + ( width - padLeft - padRight - textWidth ) / 2;
    return left + padLeft;
}

/*-------------------------------------------------------------------------*/

CString
CWidgetPainter::FitText( CGUIContext& context, const CString& font, const CString& text, Int32 maxWidth, bool& cut )
{GUCEF_TRACE;

    cut = false;
    if ( text.IsNULLOrEmpty() )
        return text;
    Int32 width = 0, height = 0;
    context.MeasureText( font, text, 0, width, height );
    if ( width <= maxWidth )
        return text;

    cut = true;
    static const CString ellipsis( "..." );
    Int32 ellipsisWidth = 0;
    context.MeasureText( font, ellipsis, 0, ellipsisWidth, height );
    if ( ellipsisWidth > maxWidth )
        return CString::Empty;

    // the longest start that fits beside the dots: a binary search over the length, never ending inside a UTF-8 sequence
    UInt32 low = 0;
    UInt32 high = text.Length();
    while ( low < high )
    {
        UInt32 middle = ( low + high + 1 ) / 2;
        context.MeasureText( font, text.SubstrFromRange( 0, middle ), 0, width, height );
        if ( width + ellipsisWidth <= maxWidth )
            low = middle;
        else
            high = middle - 1;
    }
    if ( 1 == sizeof( text[ 0 ] ) )
    {
        while ( low > 0 && low < text.Length() && 0x80 == ( (UInt32)(unsigned char) text[ low ] & 0xC0 ) )
            --low;
    }
    // no space before the dots
    while ( low > 0 && ' ' == text[ low - 1 ] )
        --low;
    return text.SubstrFromRange( 0, low ) + ellipsis;
}

/*-------------------------------------------------------------------------*/

CString
CWidgetPainter::FitText( CGUIContext& context, const CString& font, const CString& text, Int32 maxWidth, ETextOverflow mode, bool& cut )
{GUCEF_TRACE;

    cut = false;
    if ( TEXTOVERFLOW_CLIP == mode || text.IsNULLOrEmpty() )
        return text;
    if ( TEXTOVERFLOW_ELLIPSISEND == mode )
        return FitText( context, font, text, maxWidth, cut );

    Int32 width = 0, height = 0;
    context.MeasureText( font, text, 0, width, height );
    if ( width <= maxWidth )
        return text;
    cut = true;
    static const CString ellipsis( "..." );
    Int32 ellipsisWidth = 0;
    context.MeasureText( font, ellipsis, 0, ellipsisWidth, height );
    if ( ellipsisWidth > maxWidth )
        return CString::Empty;
    const Int32 room = maxWidth - ellipsisWidth;

    if ( TEXTOVERFLOW_ELLIPSISSTART == mode )
    {
        // the end of the text is kept: "...rest of the text"
        UInt32 start = LongestEnd( context, font, text, room );
        while ( start < text.Length() && ' ' == text[ start ] )
            ++start;
        return ellipsis + text.SubstrFromRange( start, text.Length() );
    }

    // the middle: the extension of a file name stays whole at the end, the start gets about 60 % of the room
    CString extension;
    const Int32 dot = text.HasChar( '.', false );
    const Int32 slash = text.HasChar( '/', false ) > text.HasChar( '\\', false ) ? text.HasChar( '/', false ) : text.HasChar( '\\', false );
    if ( dot > 0 && dot > slash + 1 && text.Length() - (UInt32) dot <= 9 )
        extension = text.SubstrFromRange( (UInt32) dot, text.Length() );
    Int32 extensionWidth = 0;
    if ( !extension.IsNULLOrEmpty() )
        context.MeasureText( font, extension, 0, extensionWidth, height );
    if ( extensionWidth > room )
    {
        extension = CString::Empty;
        extensionWidth = 0;
    }
    Int32 tailRoom = room - room * 6 / 10;
    if ( tailRoom < extensionWidth )
        tailRoom = extensionWidth;
    UInt32 tailStart = LongestEnd( context, font, text, tailRoom );
    if ( !extension.IsNULLOrEmpty() && tailStart > (UInt32) dot )
        tailStart = (UInt32) dot;
    const CString tail = text.SubstrFromRange( tailStart, text.Length() );
    Int32 tailWidth = 0;
    context.MeasureText( font, tail, 0, tailWidth, height );
    UInt32 headEnd = LongestStart( context, font, text.SubstrFromRange( 0, tailStart ), room - tailWidth );
    while ( headEnd > 0 && ' ' == text[ headEnd - 1 ] )
        --headEnd;
    return text.SubstrFromRange( 0, headEnd ) + ellipsis + tail;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetPainter::PaintText( CGUIContext& context     ,
                           const CString& font      ,
                           const CString& sourceText,
                           const CRect& area        ,
                           const CString& hAlign    ,
                           const CString& vAlign    ,
                           bool wordWrap            ,
                           const CColour& colour    ,
                           TCommandVector& commands ,
                           ETextOverflow overflow   ,
                           UInt32 textFlags         ,
                           bool displayText         )
{GUCEF_TRACE;

    if ( sourceText.IsNULLOrEmpty() )
        return false;
    const CString text = displayText ? context.GetDisplayText( sourceText ) : sourceText;

    TStringVector lines;
    TBitmapFontPtr bitmapFont = context.GetBitmapFont( font );
    if ( !bitmapFont.IsNULL() )
    {
        CBitmapFont::TStringVector broken = bitmapFont->BreakLines( text, wordWrap ? area.width : 0 );
        lines.assign( broken.begin(), broken.end() );
    }
    else
    {
        lines = SplitStringList( text );
    }

    Int32 lineHeight = context.GetLineHeight( font );
    bool elided = false;
    if ( TEXTOVERFLOW_CLIP != overflow )
    {
        // wrapped text that is higher than the area: the last line that fits ends with "..."
        if ( wordWrap && lineHeight > 0 && (Int32) lines.size() * lineHeight > area.height )
        {
            UInt32 fitting = (UInt32) ( area.height / lineHeight );
            if ( fitting < 1 )
                fitting = 1;
            if ( fitting < (UInt32) lines.size() )
            {
                lines.resize( fitting );
                bool cut = false;
                CString last = FitText( context, font, lines.back() + "...", area.width, TEXTOVERFLOW_ELLIPSISEND, cut );
                if ( !cut )
                    last = lines.back() + "...";
                lines.back() = last;
                elided = true;
            }
        }
    }

    Int32 totalHeight = lineHeight * (Int32) lines.size();
    Int32 y = area.top;
    if ( vAlign == "Center" )
        y = area.top + ( area.height - totalHeight ) / 2;
    else if ( vAlign == "Bottom" )
        y = area.Bottom() - totalHeight;

    for ( UInt32 i=0; i<(UInt32)lines.size(); ++i, y += lineHeight )
    {
        CString line = lines[ i ];
        UInt32 flags = textFlags;
        bool cut = false;
        if ( TEXTOVERFLOW_CLIP != overflow )
            line = FitText( context, font, line, area.width, overflow, cut );
        if ( cut || ( elided && i + 1 == (UInt32) lines.size() ) )
        {
            flags |= CDrawSurface::TEXTFLAG_ELIDED;
            elided = true;
        }
        Int32 width = 0, height = 0;
        context.MeasureText( font, line, 0, width, height );
        Int32 x = area.left;
        if ( hAlign == "Center" )
            x = area.left + ( area.width - width ) / 2;
        else if ( hAlign == "Right" )
            x = area.Right() - width;
        Text( commands, font, line, x, y, colour, flags, area );
    }
    return elided;
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintText( CGUIContext& context     ,
                           const CString& font      ,
                           const CString& text      ,
                           const CRect& area        ,
                           const CString& hAlign    ,
                           const CString& vAlign    ,
                           bool wordWrap            ,
                           const CColour& colour    ,
                           TCommandVector& commands )
{GUCEF_TRACE;

    PaintText( context, font, text, area, hAlign, vAlign, wordWrap, colour, commands, TEXTOVERFLOW_CLIP, CDrawSurface::TEXTFLAG_NONE, true );
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintBox( const CWidget& widget, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) ) );
    if ( widget.HasProperty( EWidgetProperty_BackgroundImage ) )
        Image( commands, widget.GetPropertyRef( EWidgetProperty_BackgroundImage ), area, CRect(), CColour( 255, 255, 255, 255 ) );
    if ( widget.HasProperty( EWidgetProperty_BorderColour ) )
        Stroke( commands, area, ColourOr( widget, EWidgetProperty_BorderColour, CColour( 0, 0, 0, 0 ) ), widget.GetIntProperty( EWidgetProperty_BorderWidth ) );
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintImage( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) ) );
    CString image = widget.GetPropertyRef( EWidgetProperty_Image );
    Int32 imageWidth = 0, imageHeight = 0;
    if ( image.IsNULLOrEmpty() || !context.GetImageLogicalSize( image, imageWidth, imageHeight ) )
        return;

    CRect source = widget.GetRectProperty( EWidgetProperty_SubRect );
    Int32 w = source.IsEmpty() ? imageWidth : source.width;
    Int32 h = source.IsEmpty() ? imageHeight : source.height;
    CRect target = FitImage( w, h, area, widget.GetPropertyRef( EWidgetProperty_ScaleMode ) );
    Image( commands, image, target, source, ColourOr( widget, EWidgetProperty_Tint, CColour( 255, 255, 255, 255 ) ) );
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintButton( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    const CButton* button = AsType< CButton >( widget );
    CString image = GUCEF_NULL != button ? button->GetCurrentImage() : CString::Empty;
    UInt32 state = widget.GetState();
    bool checkbox = IsA( widget, "Checkbox" );

    if ( !image.IsNULLOrEmpty() )
    {
        // a picture button: the picture of the state is the button
        Image( commands, image, area, CRect(), CColour( 255, 255, 255, 255 ) );
    }
    else
    if ( checkbox )
    {
        // a box on the left with a mark when checked, the text next to it
        Int32 size = area.height < 14 ? area.height : 14;
        CRect box( area.left, area.top + ( area.height - size ) / 2, size, size );
        Fill( commands, box, FieldColour );
        Stroke( commands, box, 0 != ( state & CWidget::STATE_HOVERED ) ? FocusColour : BoxBorderColour, 1 );
        if ( 0 != ( state & CWidget::STATE_CHECKED ) )
            Fill( commands, CRect( box.left + 3, box.top + 3, size - 6, size - 6 ), TextColourOf( widget ) );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Text ), CRect( area.left + size + 4, area.top, area.width - size - 4, area.height ),
                   "Left", "Center", false, TextColourOf( widget ), commands, OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ) );
        return;
    }
    else
    {
        CColour background = ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) );
        if ( 0 == background.a )
        {
            if ( 0 != ( state & CWidget::STATE_PRESSED ) && 0 != ( state & CWidget::STATE_HOVERED ) )
                background = BoxPressedColour;
            else if ( 0 != ( state & CWidget::STATE_HOVERED ) )
                background = BoxHoverColour;
            else
                background = BoxColour;
            Fill( commands, area, background );
            Stroke( commands, area, BoxBorderColour, 1 );
        }
        else
        {
            Fill( commands, area, background );
        }
    }

    if ( 0 != ( state & CWidget::STATE_FOCUSED ) && image.IsNULLOrEmpty() )
        Stroke( commands, CRect( area.left + 1, area.top + 1, area.width - 2, area.height - 2 ), FocusColour, 1 );

    // the text inside the padding of the button, shortened with "..." when it does not fit (its tooltip is then the whole text)
    const CRect textArea( area.left + ButtonTextPadding, area.top, area.width - 2 * ButtonTextPadding > 0 ? area.width - 2 * ButtonTextPadding : 0, area.height );
    PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Text ), textArea, "Center", "Center", false, TextColourOf( widget ), commands,
               OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ) );
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintEditbox( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, FieldColour ) );
    if ( !widget.IsPropertySet( EWidgetProperty_BackgroundColour ) )
        Stroke( commands, area, widget.HasFocus() ? FocusColour : BoxBorderColour, 1 );

    CString font = widget.GetPropertyRef( EWidgetProperty_Font );
    CString text = widget.GetPropertyRef( EWidgetProperty_Text );

    // a secret input draws the mask characters its text property holds; only while the value is revealed (or the code point typed last is shown) it says more
    const CSecretInput* secret = CWidget::As< CSecretInput >( &widget );
    const bool clearText = GUCEF_NULL != secret && secret->IsDisplayingClearText();
    if ( clearText )
        secret->GetDisplayText( text );

    Int32 lineHeight = context.GetLineHeight( font );
    Int32 width = 0, height = 0;
    context.MeasureText( font, text, 0, width, height );
    Int32 x = area.left + 2;
    Int32 y = area.top + ( area.height - lineHeight ) / 2;

    PushClip( commands, area );
    if ( widget.GetBoolProperty( EWidgetProperty_Selected ) && width > 0 )
        Fill( commands, CRect( x, y, width, lineHeight ), ColourOr( widget, EWidgetProperty_SelectionColour, CColour( 60, 90, 160, 255 ) ) );
    Text( commands, font, text, x, y, TextColourOf( widget ), CDrawSurface::TEXTFLAG_SCROLLABLE_X, area );
    if ( widget.GetBoolProperty( EWidgetProperty_CaretVisible ) )
        Fill( commands, CRect( x + width, y, 1, lineHeight ), TextColourOf( widget ) );
    PopClip( commands );
    if ( clearText )
        CORE::SecureWipeString( text );
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintRows( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    CString font = widget.GetPropertyRef( EWidgetProperty_Font );
    CColour textColour = ColourOr( widget, EWidgetProperty_TextColour, CColour( 255, 255, 255, 255 ) );
    CColour disabledColour = ColourOr( widget, EWidgetProperty_DisabledTextColour, DisabledTextColour );
    CColour selectionColour = ColourOr( widget, EWidgetProperty_SelectionColour, CColour( 0, 0, 0, 0 ) );
    CColour hoverColour = ColourOr( widget, EWidgetProperty_HoverColour, CColour( 0, 0, 0, 0 ) );
    // the row of a menu under the pointer is always marked: the look of the hovered box when nothing says otherwise
    if ( 0 == hoverColour.a && IsA( widget, "PopupMenu" ) )
        hoverColour = BoxHoverColour;
    Int32 hoverIndex = widget.GetIntProperty( EWidgetProperty_HoverIndex );
    Int32 scroll = widget.GetIntProperty( EWidgetProperty_ScrollPosition );

    PushClip( commands, area );

    if ( const CListbox* list = AsType< CListbox >( widget ) )
    {
        Int32 rowHeight = list->GetRowHeight();
        CListbox::TStringVector items = list->GetItems();
        Int32 lineHeight = context.GetLineHeight( font );

        // rows with a colour of their own: a marker of it at the left and the text in it; then every row makes room for the marker
        const bool marked = list->HasItemColours();
        const Int32 textLeft = area.left + CListbox::ITEM_PADDING_LEFT + ( marked ? (Int32) CListbox::ITEM_MARKER_ROOM : 0 );
        const bool ellipsis = "Ellipsis" == widget.GetPropertyRef( EWidgetProperty_TextOverflow );
        const Int32 room = list->GetItemTextRoom();
        for ( Int32 row = scroll, y = area.top; row < (Int32) items.size() && y < area.Bottom(); ++row, y += rowHeight )
        {
            CRect rowRect( area.left, y, area.width, rowHeight );
            if ( list->IsItemSelected( row ) )
                Fill( commands, rowRect, selectionColour );
            CColour rowColour = textColour;
            CColour itemColour;
            if ( marked && list->GetItemColour( row, itemColour ) )
            {
                Fill( commands, CRect( area.left + CListbox::ITEM_PADDING_LEFT, y + 3, CListbox::ITEM_MARKER_WIDTH, rowHeight - 6 ), itemColour );
                rowColour = itemColour;
            }
            bool cut = false;
            const CString shown = context.GetDisplayText( items[ row ] );
            const CString text = ellipsis ? FitText( context, font, shown, room, cut ) : shown;
            Text( commands, font, text, textLeft, y + ( rowHeight - lineHeight ) / 2, list->IsItemDisabled( row ) ? disabledColour : rowColour,
                  CDrawSurface::TEXTFLAG_SCROLLABLE_Y | ( cut ? CDrawSurface::TEXTFLAG_ELIDED : 0 ), rowRect );
            if ( row == hoverIndex && !list->IsItemDisabled( row ) )
                Stroke( commands, rowRect, hoverColour, 1 );
        }
    }
    else
    if ( const CTreeview* tree = AsType< CTreeview >( widget ) )
    {
        Int32 rowHeight = tree->GetRowHeight();
        Int32 indent = widget.GetIntProperty( EWidgetProperty_Indent );
        CTreeview::TRowVector rows = tree->GetVisibleRows();
        CString selected = tree->GetSelectedItem();
        const CTreeview::TStringVector badges = widget.GetStringListProperty( EWidgetProperty_ItemBadges );
        for ( Int32 row = scroll, y = area.top; row < (Int32) rows.size() && y < area.Bottom(); ++row, y += rowHeight )
        {
            const CTreeview::TRow& entry = rows[ row ];
            CRect rowRect( area.left, y, area.width, rowHeight );
            if ( entry.path == selected )
                Fill( commands, rowRect, selectionColour );
            Int32 x = area.left + entry.depth * indent;
            if ( entry.hasChildren )
            {
                // a plus or a minus in the indent column
                Int32 cx = x + indent / 2;
                Int32 cy = y + rowHeight / 2;
                Line( commands, cx - 3, cy, cx + 3, cy, textColour );
                if ( !entry.expanded )
                    Line( commands, cx, cy - 3, cx, cy + 3, textColour );
            }
            // a badge (ItemBadges) at the right end of the row: a dot or a pill; the label is cut off before it
            const CString badge = badges.empty() ? CString::Empty : CTreeview::FindItemBadge( badges, entry.path );
            const UInt32 rowFlags = CDrawSurface::TEXTFLAG_SCROLLABLE_Y;
            if ( badge.IsNULLOrEmpty() )
                Text( commands, font, context.GetDisplayText( entry.label ), x + indent, y, textColour, rowFlags, rowRect );
            else
            {
                CRect badgeRect;
                if ( "*" == badge )
                    badgeRect = CRect( area.Right() - 4 - 8, y + ( rowHeight - 8 ) / 2, 8, 8 );
                else
                {
                    Int32 badgeWidth = 0, badgeHeight = 0;
                    context.MeasureText( font, badge, 0, badgeWidth, badgeHeight );
                    Int32 pillHeight = context.GetLineHeight( font ) + 2;
                    if ( pillHeight > rowHeight - 2 )
                        pillHeight = rowHeight - 2;
                    badgeRect = CRect( area.Right() - 4 - ( badgeWidth + 8 ), y + ( rowHeight - pillHeight ) / 2, badgeWidth + 8, pillHeight );
                }
                PushClip( commands, CRect( area.left, y, badgeRect.left - 4 - area.left > 0 ? badgeRect.left - 4 - area.left : 0, rowHeight ) );
                Text( commands, font, context.GetDisplayText( entry.label ), x + indent, y, textColour, rowFlags, rowRect );
                PopClip( commands );
                Fill( commands, badgeRect, AccentColour );
                if ( !( "*" == badge ) )
                    PaintText( context, font, badge, badgeRect, "Center", "Center", false, CColour( 255, 255, 255, 255 ), commands );
            }
            if ( row == hoverIndex )
                Stroke( commands, rowRect, hoverColour, 1 );
        }
    }
    else
    if ( const CTextbox* textbox = AsType< CTextbox >( widget ) )
    {
        Int32 rowHeight = context.GetLineHeight( font );
        if ( widget.GetIntProperty( EWidgetProperty_RowHeight ) > 0 )
            rowHeight = widget.GetIntProperty( EWidgetProperty_RowHeight );
        CTextbox::TStringVector lines = textbox->GetDisplayLines();
        for ( Int32 line = scroll, y = area.top; line < (Int32) lines.size() && y < area.Bottom(); ++line, y += rowHeight )
            Text( commands, font, context.GetDisplayText( lines[ line ] ), area.left, y, textColour, CDrawSurface::TEXTFLAG_SCROLLABLE_Y, CRect( area.left, y, area.width, rowHeight ) );
    }
    else
    if ( const CGridView* grid = AsType< CGridView >( widget ) )
    {
        const size_t first = commands.size();
        CGridViewPainter::Paint( *grid, context, area, commands );

        // the rows scroll, and the columns when the grid scrolls sideways: a cell cut at the edge of the view is scrolled to
        const UInt32 flags = CDrawSurface::TEXTFLAG_SCROLLABLE_Y | ( grid->IsHorizontallyScrollable() ? CDrawSurface::TEXTFLAG_SCROLLABLE_X : 0 );
        for ( size_t i=first; i<commands.size(); ++i )
            if ( CDrawSurface::COMMAND_TEXT == commands[ i ].type )
                commands[ i ].textFlags |= flags;
    }
    else
    if ( const CPopupMenu* menu = AsType< CPopupMenu >( widget ) )
    {
        Int32 rowHeight = menu->GetRowHeight();
        CPopupMenu::TStringVector items = menu->GetItems();
        for ( Int32 row = 0, y = area.top; row < (Int32) items.size(); ++row, y += rowHeight )
        {
            CRect rowRect( area.left, y, area.width, rowHeight );
            if ( items[ row ] == "-" )
            {
                Line( commands, area.left + 2, y + rowHeight / 2, area.Right() - 3, y + rowHeight / 2, disabledColour );
                continue;
            }
            if ( row == hoverIndex )
                Fill( commands, rowRect, hoverColour );
            Text( commands, font, context.GetDisplayText( items[ row ] ), area.left + 8, y, menu->IsItemSelectable( row ) ? textColour : disabledColour,
                  CDrawSurface::TEXTFLAG_NONE, rowRect );
        }
    }

    PopClip( commands );
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::Paint( const CWidget& widget, CGUIContext& context, TCommandVector& commands )
{GUCEF_TRACE;

    const CRect& rect = widget.GetRect();
    CRect area( 0, 0, rect.width, rect.height );
    if ( area.IsEmpty() )
        return;

    if ( IsA( widget, "Button" ) )
    {
        PaintButton( widget, context, area, commands );
        return;
    }
    if ( IsA( widget, "ImageFrame" ) )
    {
        PaintImage( widget, context, area, commands );
        return;
    }
    if ( IsA( widget, "Label" ) )
    {
        PaintBox( widget, area, commands );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Text ), area, widget.GetPropertyRef( EWidgetProperty_HTextAlign ),
                   widget.GetPropertyRef( EWidgetProperty_VTextAlign ), widget.GetBoolProperty( EWidgetProperty_WordWrap ), TextColourOf( widget ), commands,
                   OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ) );
        return;
    }
    if ( IsA( widget, "Editbox" ) )
    {
        PaintEditbox( widget, context, area, commands );
        return;
    }
    if ( IsA( widget, "Listbox" ) || IsA( widget, "Treeview" ) || IsA( widget, "Textbox" ) || IsA( widget, "GridView" ) || IsA( widget, "PopupMenu" ) )
    {
        // a menu, and a list that is open as the popup of the context (the list of a combo box), is drawn over other widgets:
        // it is never see-through, whatever its colours say, and it has a border
        const bool popup = IsA( widget, "PopupMenu" ) || context.GetPopup() == &widget;
        CColour background = ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) );
        if ( popup && 0 == background.a )
            background = BoxColour;
        else
        if ( popup && background.a < 255 )
            background.a = 255;
        Fill( commands, area, background );
        PaintRows( widget, context, area, commands );
        if ( popup )
        {
            CColour border = ColourOr( widget, EWidgetProperty_BorderColour, BoxBorderColour );
            if ( 0 == border.a )
                border = BoxBorderColour;
            Stroke( commands, area, border, 1 );
            return;
        }
        // the list that takes the keys says so, as a field does
        if ( widget.HasFocus() && ( IsA( widget, "Listbox" ) || IsA( widget, "Treeview" ) || IsA( widget, "GridView" ) ) )
            Stroke( commands, area, FocusColour, 1 );
        return;
    }
    if ( IsA( widget, "DrawSurface" ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) ) );
        const CDrawSurface& surface = static_cast< const CDrawSurface& >( widget );
        bool clip = widget.GetBoolProperty( EWidgetProperty_Clip );
        if ( clip )
            PushClip( commands, area );
        commands.insert( commands.end(), surface.GetCommands().begin(), surface.GetCommands().end() );
        if ( clip )
            PopClip( commands );
        return;
    }
    if ( IsA( widget, "Combobox" ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, FieldColour ) );
        Stroke( commands, area, widget.HasFocus() ? FocusColour : BoxBorderColour, 1 );
        CRect arrow( area.Right() - area.height, area.top, area.height, area.height );
        DownArrow( commands, arrow, TextColourOf( widget ) );
        PushClip( commands, CRect( area.left, area.top, area.width - area.height, area.height ) );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Text ), CRect( area.left + 2, area.top, area.width - area.height - 2, area.height ),
                   "Left", "Center", false, TextColourOf( widget ), commands, OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ) );
        PopClip( commands );
        return;
    }
    if ( const CSpinner* spinner = AsType< CSpinner >( widget ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, FieldColour ) );
        Stroke( commands, area, widget.HasFocus() ? FocusColour : BoxBorderColour, 1 );
        Int32 part = area.width / 4;
        CColour colour = TextColourOf( widget );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), "-", CRect( 0, 0, part, area.height ), "Center", "Center", false, colour, commands );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), "+", CRect( area.width - part, 0, part, area.height ), "Center", "Center", false, colour, commands );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), spinner->GetValueText(), area, "Center", "Center", false, colour, commands,
                   TEXTOVERFLOW_CLIP, CDrawSurface::TEXTFLAG_NUMERIC, false );
        return;
    }
    if ( const CSlider* slider = AsType< CSlider >( widget ) )
    {
        CString trackImage = widget.GetPropertyRef( EWidgetProperty_TrackImage );
        if ( !trackImage.IsNULLOrEmpty() )
            Image( commands, trackImage, area, CRect(), CColour( 255, 255, 255, 255 ) );
        else
            Fill( commands, area, ColourOr( widget, EWidgetProperty_TrackColour, BoxColour ) );
        CRect thumb = slider->GetThumbRect();
        CString thumbImage = widget.GetPropertyRef( EWidgetProperty_ThumbImage );
        if ( !thumbImage.IsNULLOrEmpty() )
            Image( commands, thumbImage, thumb, CRect(), CColour( 255, 255, 255, 255 ) );
        else
            Fill( commands, thumb, ColourOr( widget, EWidgetProperty_ThumbColour, BoxBorderColour ) );
        if ( widget.HasFocus() )
            Stroke( commands, area, FocusColour, 1 );
        return;
    }
    if ( const CProgressBar* progress = AsType< CProgressBar >( widget ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, FieldColour ) );
        CRect bar( 0, 0, (Int32) ( (Float32) area.width * progress->GetFraction() + 0.5f ), area.height );
        CString barImage = widget.GetPropertyRef( EWidgetProperty_BarImage );
        if ( !barImage.IsNULLOrEmpty() )
            Image( commands, barImage, bar, CRect(), CColour( 255, 255, 255, 255 ) );
        else
            Fill( commands, bar, ColourOr( widget, EWidgetProperty_BarColour, CColour( 60, 140, 60, 255 ) ) );
        return;
    }
    if ( const CTabControl* tabs = AsType< CTabControl >( widget ) )
    {
        // the tabs, their badges and the overflow button of TabSizing Content
        CTabControlPainter::Paint( *tabs, context, area, commands );
        return;
    }
    if ( const CMenuBar* bar = AsType< CMenuBar >( widget ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, BoxColour ) );
        CMenuBar::TStringVector menus = bar->GetMenus();
        Int32 open = widget.GetIntProperty( EWidgetProperty_OpenMenu );
        Int32 hover = widget.GetIntProperty( EWidgetProperty_HoverIndex );
        for ( UInt32 i=0; i<(UInt32)menus.size(); ++i )
        {
            CRect title = bar->GetMenuRect( i );
            if ( (Int32) i == open || (Int32) i == hover )
                Fill( commands, title, ColourOr( widget, EWidgetProperty_HoverColour, BoxHoverColour ) );
            PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), menus[ i ], title, "Center", "Center", false, TextColourOf( widget ), commands );
        }
        return;
    }
    if ( const CAudioPlayer* player = AsType< CAudioPlayer >( widget ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, FieldColour ) );
        CColour colour = ColourOr( widget, EWidgetProperty_BarColour, CColour( 60, 140, 60, 255 ) );
        Int32 size = area.height - 6;
        if ( size < 4 )
            size = 4;
        CRect symbol( 3, ( area.height - size ) / 2, size, size );
        if ( player->IsPlaying() && !player->IsPaused() )
        {
            // pause: two bars
            Fill( commands, CRect( symbol.left, symbol.top, size / 3, size ), colour );
            Fill( commands, CRect( symbol.Right() - size / 3, symbol.top, size / 3, size ), colour );
        }
        else
        {
            // play: a triangle pointing right, as lines
            for ( Int32 i=0; i<size; ++i )
            {
                Int32 half = i < size / 2 ? i : size - 1 - i;
                Line( commands, symbol.left, symbol.top + i, symbol.left + half, symbol.top + i, colour );
            }
        }
        Int32 seconds = widget.GetIntProperty( EWidgetProperty_PositionInMs ) / 1000;
        char time[ 32 ];
        snprintf( time, sizeof( time ), "%d:%02d", seconds / 60, seconds % 60 );
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), time, CRect( symbol.Right() + 4, 0, area.width - symbol.Right() - 4, area.height ),
                   "Left", "Center", false, TextColourOf( widget ), commands, TEXTOVERFLOW_CLIP, CDrawSurface::TEXTFLAG_NUMERIC, false );
        return;
    }

    // panels, forms, the containers, windows and plain widgets
    PaintBox( widget, area, commands );
    if ( IsA( widget, "GroupBox" ) )
    {
        // the frame; its top line goes through the middle of the caption, with a gap where the caption is
        CString caption = widget.GetPropertyRef( EWidgetProperty_Caption );
        CString font = widget.GetPropertyRef( EWidgetProperty_Font );
        Int32 captionHeight = caption.IsNULLOrEmpty() ? 0 : context.GetLineHeight( font );
        Int32 lineY = captionHeight / 2;
        Int32 right = area.width - 1;
        Int32 bottom = area.height - 1;
        CColour frame = ColourOr( widget, EWidgetProperty_FrameColour, CColour( 120, 120, 130, 255 ) );
        Line( commands, 0, lineY, 0, bottom, frame );
        Line( commands, right, lineY, right, bottom, frame );
        Line( commands, 0, bottom, right, bottom, frame );
        if ( captionHeight > 0 )
        {
            // the caption as shown, shortened with "..." when it is wider than the frame
            const Int32 captionRoom = area.width - 2 * ( 8 + 4 ) > 0 ? area.width - 2 * ( 8 + 4 ) : 0;
            bool captionCut = false;
            caption = FitText( context, font, context.GetDisplayText( caption ), captionRoom, OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ), captionCut );
            Int32 textWidth = 0, textHeight = 0;
            context.MeasureText( font, caption, 0, textWidth, textHeight );
            const Int32 gapLeft = 8;
            const Int32 gapRight = gapLeft + 4 + textWidth + 4;
            Line( commands, 0, lineY, gapLeft, lineY, frame );
            if ( gapRight < right )
                Line( commands, gapRight, lineY, right, lineY, frame );
            PaintText( context, font, caption, CRect( gapLeft + 4, 0, textWidth + 1, captionHeight ), "Left", "Center", false, TextColourOf( widget ), commands,
                       TEXTOVERFLOW_CLIP, captionCut ? CDrawSurface::TEXTFLAG_ELIDED : CDrawSurface::TEXTFLAG_NONE, false );
        }
        else
        {
            Line( commands, 0, 0, right, 0, frame );
        }
    }
    if ( IsA( widget, "Window" ) && widget.GetBoolProperty( EWidgetProperty_TitleVisible ) )
    {
        Int32 titleHeight = widget.GetIntProperty( EWidgetProperty_TitleHeight );
        CRect title( 0, 0, area.width, titleHeight );
        CColour titleColour = ColourOr( widget, EWidgetProperty_TitleColour, CColour( 255, 255, 255, 255 ) );
        Fill( commands, title, ColourOr( widget, EWidgetProperty_TitleBarColour, BoxColour ) );

        // the fold button at the right end of the title bar, with the title in what is left
        Int32 textRight = area.width - 8;
        const CWindow* window = CWidget::ToWindow( &widget );
        CRect fold = GUCEF_NULL != window ? window->GetFoldButtonRect() : CRect();
        if ( !fold.IsEmpty() )
        {
            textRight = fold.left - 4;
            if ( widget.GetBoolProperty( EWidgetProperty_FoldButtonPressed ) )
                Fill( commands, fold, ColourOr( widget, EWidgetProperty_FoldButtonPressedColour, CColour( 255, 255, 255, 60 ) ) );
            else
            if ( widget.GetBoolProperty( EWidgetProperty_FoldButtonHovered ) )
                Fill( commands, fold, ColourOr( widget, EWidgetProperty_FoldButtonHoverColour, CColour( 255, 255, 255, 30 ) ) );
            FoldChevron( commands, fold, window->IsCollapsed(), titleColour );
        }
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Title ), CRect( 10, 0, textRight - 10 > 0 ? textRight - 10 : 0, titleHeight ),
                   "Left", "Center", false, titleColour, commands, OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ) );
    }
    if ( const CSplitter* splitter = AsType< CSplitter >( widget ) )
    {
        // the handle between the panes with three grip dots; lighter under the pointer and while it is dragged
        const CRect handle = splitter->GetHandleRect();
        if ( !handle.IsEmpty() )
        {
            Fill( commands, handle, splitter->IsHandleActive() ? ColourOr( widget, EWidgetProperty_HoverColour, CColour( 86, 124, 196, 255 ) ) : ColourOr( widget, EWidgetProperty_HandleColour, CColour( 52, 56, 68, 255 ) ) );
            const CColour grip( 170, 176, 192, 255 );
            const bool sideBySide = splitter->IsSideBySide();
            const Int32 cx = handle.left + handle.width / 2;
            const Int32 cy = handle.top + handle.height / 2;
            const Int32 thickness = ( sideBySide ? handle.width : handle.height ) >= 4 ? 2 : 1;
            for ( Int32 dot=-1; dot<=1; ++dot )
            {
                const Int32 offset = dot * 5;
                Fill( commands, sideBySide ? CRect( cx - thickness / 2, cy + offset, thickness, thickness ) : CRect( cx + offset, cy - thickness / 2, thickness, thickness ), grip );
            }
        }
    }
    if ( const CFoldablePanel* foldable = AsType< CFoldablePanel >( widget ) )
    {
        // the header with the title and the fold button; folded it is the strip, a vertical one with the title a letter to a line
        const CRect header = foldable->GetHeaderRect();
        const CColour titleColour = ColourOr( widget, EWidgetProperty_TitleColour, CColour( 220, 224, 234, 255 ) );
        Fill( commands, header, ColourOr( widget, EWidgetProperty_TitleBarColour, CColour( 40, 44, 56, 255 ) ) );
        const CRect button = foldable->GetFoldButtonRect();
        if ( widget.GetBoolProperty( EWidgetProperty_FoldButtonHovered ) )
            Fill( commands, button, ColourOr( widget, EWidgetProperty_FoldButtonHoverColour, CColour( 255, 255, 255, 30 ) ) );
        // the chevron points at the edge the panel folds into, and away from it while it is folded
        const CString& dock = widget.GetPropertyRef( EWidgetProperty_Dock );
        const bool folded = foldable->IsFolded();
        const Int32 cx = button.left + button.width / 2;
        const Int32 cy = button.top + button.height / 2;
        for ( Int32 thick=0; thick<2; ++thick )
        {
            if ( dock == "Left" || dock == "Right" )
            {
                const Int32 direction = ( ( dock == "Left" ) != folded ) ? -1 : 1;
                Line( commands, cx - direction * 2 + thick, cy - 5, cx + direction * 2 + thick, cy, titleColour );
                Line( commands, cx + direction * 2 + thick, cy, cx - direction * 2 + thick, cy + 5, titleColour );
            }
            else
            {
                const Int32 direction = ( ( dock == "Bottom" ) != folded ) ? 1 : -1;
                Line( commands, cx - 5, cy + direction * 2 + thick, cx, cy - direction * 2 + thick, titleColour );
                Line( commands, cx, cy - direction * 2 + thick, cx + 5, cy + direction * 2 + thick, titleColour );
            }
        }
        const CString& title = widget.GetPropertyRef( EWidgetProperty_Title );
        const CString font = widget.GetPropertyRef( EWidgetProperty_Font );
        if ( folded && foldable->IsStripVertical() )
        {
            const Int32 lineHeight = context.GetLineHeight( font );
            Int32 y = button.Bottom() + 4;
            for ( UInt32 i=0; i<title.Length() && lineHeight > 0 && y + lineHeight <= header.Bottom(); ++i, y += lineHeight )
            {
                const CString letter = title.SubstrFromRange( i, i + 1 );
                PaintText( context, font, letter, CRect( 0, y, header.width, lineHeight ), "Center", "Center", false, titleColour, commands );
            }
        }
        else
        {
            const Int32 textLeft = button.left < 8 ? button.Right() + 4 : 10;
            const Int32 textRight = button.left < 8 ? header.width - 8 : button.left - 4;
            if ( textRight > textLeft )
                PaintText( context, font, title, CRect( textLeft, header.top, textRight - textLeft, header.height ), "Left", "Center", false, titleColour, commands,
                           OverflowOf( widget, TEXTOVERFLOW_ELLIPSISEND ) );
        }
    }
    if ( const CScrollPanel* scrollPanel = CWidget::ToScrollPanel( &widget ) )
    {
        // the scroll bar: a track the full height at the right end of the column and, when the content does not fit, the thumb
        // inside it, a few pixels from its edges; the whole width of the track is where the pointer grabs it
        CRect track = scrollPanel->GetTrackRect();
        if ( scrollPanel->IsScrollable() && !track.IsEmpty() )
        {
            Fill( commands, track, ColourOr( widget, EWidgetProperty_TrackColour, FieldColour ) );
            Fill( commands, CRect( track.left, track.top, 1, track.height ), ColourOr( widget, EWidgetProperty_TrackEdgeColour, CColour( 58, 64, 78, 255 ) ) );
            CRect thumb = scrollPanel->GetThumbRect();
            thumb = CRect( thumb.left + 2, thumb.top + 2, thumb.width > 4 ? thumb.width - 4 : thumb.width, thumb.height > 4 ? thumb.height - 4 : thumb.height );
            EWidgetProperty colourName = EWidgetProperty_ThumbColour;
            if ( scrollPanel->IsThumbDragged() )
                colourName = EWidgetProperty_ThumbPressedColour;
            else
            if ( widget.GetBoolProperty( EWidgetProperty_ThumbHovered ) )
                colourName = EWidgetProperty_ThumbHoverColour;
            Fill( commands, thumb, ColourOr( widget, colourName, BoxBorderColour ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CWidgetPainter::IsOwnTextElided( const CWidget& widget, CGUIContext& context, CString& fullText )
{GUCEF_TRACE;

    // the widgets that show one text of their own; lists, grids and tabs answer for their parts themselves (GetTooltipAt())
    const bool label = IsA( widget, "Label" );
    const bool button = IsA( widget, "Button" );
    const bool combo = IsA( widget, "Combobox" );
    const bool group = IsA( widget, "GroupBox" );
    const bool window = IsA( widget, "Window" );
    if ( !label && !button && !combo && !group && !window )
        return false;
    const CString& text = widget.GetPropertyRef( group ? EWidgetProperty_Caption : ( window ? EWidgetProperty_Title : EWidgetProperty_Text ) );
    if ( text.IsNULLOrEmpty() )
        return false;

    // painted as the painter paints it, so the answer is what the user sees
    TCommandVector commands;
    Paint( widget, context, commands );
    for ( UInt32 i=0; i<(UInt32) commands.size(); ++i )
    {
        if ( CDrawSurface::COMMAND_TEXT == commands[ i ].type && 0 != ( commands[ i ].textFlags & CDrawSurface::TEXTFLAG_ELIDED ) )
        {
            fullText = text;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWidgetPainter::PaintOverlay( CGUIContext& context, TCommandVector& commands )
{GUCEF_TRACE;

    // the tooltip of the part of the widget the pointer rests on, when it rested for the delay of the tooltip service
    CTooltipService& tooltips = context.GetTooltipService();
    bool showing = false;
    CWidget* hovered = context.GetHovered();
    if ( GUCEF_NULL != hovered && tooltips.IsDue( context.GetTimeInMs() ) )
    {
        const CRect& hoveredRect = hovered->GetRect();
        const CString tooltip = tooltips.GetTooltipText( *hovered, context.GetPointerX() - hoveredRect.left, context.GetPointerY() - hoveredRect.top );
        if ( !tooltip.IsNULLOrEmpty() )
        {
            const CString font = hovered->GetPropertyRef( EWidgetProperty_Font );
            CTooltipLayout layout;
            if ( layout.Compute( context, font, tooltip, context.GetPointerX(), context.GetPointerY(), tooltips ) )
            {
                CTooltipPainter::Paint( layout, font, tooltips, commands );
                showing = true;
            }
        }
    }
    tooltips.SetShowing( showing );

    // the findings of the layout lint over everything, when its debug overlay is switched on
    CLayoutDebugOverlay& lintOverlay = context.GetLayoutDebugOverlay();
    if ( lintOverlay.IsOn() )
        lintOverlay.Paint( context, commands );

    // the cursor picture, not while a layer above takes the pointer (it has the pointer then)
    if ( context.IsCursorVisible() && !context.GetCursorImage().IsNULLOrEmpty() && !context.HasHigherLayerTakingPointerInput() )
    {
        Int32 cursorWidth = 0, cursorHeight = 0;
        if ( context.GetImageLogicalSize( context.GetCursorImage(), cursorWidth, cursorHeight ) )
        {
            CRect target( context.GetPointerX() - context.GetCursorHotspotX(), context.GetPointerY() - context.GetCursorHotspotY(),
                          cursorWidth, cursorHeight );
            Image( commands, context.GetCursorImage(), target, CRect(), CColour( 255, 255, 255, 255 ) );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
