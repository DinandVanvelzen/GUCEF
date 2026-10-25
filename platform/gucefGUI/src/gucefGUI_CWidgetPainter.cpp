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
#include "gucefGUI_CTextbox.h"
#include "gucefGUI_CListBox.h"
#include "gucefGUI_CCombobox.h"
#include "gucefGUI_CSpinner.h"
#include "gucefGUI_CSlider.h"
#include "gucefGUI_CProgressBar.h"
#include "gucefGUI_CDrawSurface.h"
#include "gucefGUI_CTabControl.h"
#include "gucefGUI_CTreeview.h"
#include "gucefGUI_CMenuBar.h"
#include "gucefGUI_CPopupMenu.h"
#include "gucefGUI_CGridView.h"
#include "gucefGUI_CAudioPlayer.h"
#include "gucefGUI_CWindow.h"
#include "gucefGUI_CScrollPanel.h"

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

/** How long the pointer rests on a widget before its tooltip shows */
const UInt64 TooltipDelayInMs = 600;

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
Text( CWidgetPainter::TCommandVector& commands, const CString& font, const CString& text, Int32 x, Int32 y, const CColour& colour )
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
    commands.push_back( command );
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

/** A small arrow pointing up, for an ascending sort in a header */
void
UpArrow( CWidgetPainter::TCommandVector& commands, const CRect& box, const CColour& colour )
{
    Int32 cx = box.left + box.width / 2;
    Int32 cy = box.top + box.height / 2;
    for ( Int32 i=0; i<4; ++i )
        Line( commands, cx - 3 + i, cy + 2 - i, cx + 3 - i, cy + 2 - i, colour );
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

    if ( text.IsNULLOrEmpty() )
        return;

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
    Int32 totalHeight = lineHeight * (Int32) lines.size();
    Int32 y = area.top;
    if ( vAlign == "Center" )
        y = area.top + ( area.height - totalHeight ) / 2;
    else if ( vAlign == "Bottom" )
        y = area.Bottom() - totalHeight;

    for ( UInt32 i=0; i<(UInt32)lines.size(); ++i, y += lineHeight )
    {
        Int32 width = 0, height = 0;
        context.MeasureText( font, lines[ i ], 0, width, height );
        Int32 x = area.left;
        if ( hAlign == "Center" )
            x = area.left + ( area.width - width ) / 2;
        else if ( hAlign == "Right" )
            x = area.Right() - width;
        Text( commands, font, lines[ i ], x, y, colour );
    }
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
                   "Left", "Center", false, TextColourOf( widget ), commands );
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

    PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Text ), area, "Center", "Center", false, TextColourOf( widget ), commands );
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
    Int32 lineHeight = context.GetLineHeight( font );
    Int32 width = 0, height = 0;
    context.MeasureText( font, text, 0, width, height );
    Int32 x = area.left + 2;
    Int32 y = area.top + ( area.height - lineHeight ) / 2;

    PushClip( commands, area );
    if ( widget.GetBoolProperty( EWidgetProperty_Selected ) && width > 0 )
        Fill( commands, CRect( x, y, width, lineHeight ), ColourOr( widget, EWidgetProperty_SelectionColour, CColour( 60, 90, 160, 255 ) ) );
    Text( commands, font, text, x, y, TextColourOf( widget ) );
    if ( widget.GetBoolProperty( EWidgetProperty_CaretVisible ) )
        Fill( commands, CRect( x + width, y, 1, lineHeight ), TextColourOf( widget ) );
    PopClip( commands );
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
    Int32 hoverIndex = widget.GetIntProperty( EWidgetProperty_HoverIndex );
    Int32 scroll = widget.GetIntProperty( EWidgetProperty_ScrollPosition );

    PushClip( commands, area );

    if ( const CListbox* list = AsType< CListbox >( widget ) )
    {
        Int32 rowHeight = list->GetRowHeight();
        CListbox::TStringVector items = list->GetItems();
        Int32 lineHeight = context.GetLineHeight( font );
        for ( Int32 row = scroll, y = area.top; row < (Int32) items.size() && y < area.Bottom(); ++row, y += rowHeight )
        {
            CRect rowRect( area.left, y, area.width, rowHeight );
            if ( list->IsItemSelected( row ) )
                Fill( commands, rowRect, selectionColour );
            Text( commands, font, items[ row ], area.left + 2, y + ( rowHeight - lineHeight ) / 2, list->IsItemDisabled( row ) ? disabledColour : textColour );
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
            Text( commands, font, entry.label, x + indent, y, textColour );
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
            Text( commands, font, lines[ line ], area.left, y, textColour );
    }
    else
    if ( const CGridView* grid = AsType< CGridView >( widget ) )
    {
        Int32 rowHeight = grid->GetRowHeight();
        GUCEF::vector< Int32 > widths = grid->GetColumnWidths();
        CGridView::TStringVector columns = widget.GetStringListProperty( EWidgetProperty_Columns );
        Fill( commands, CRect( area.left, area.top, area.width, rowHeight ), ColourOr( widget, EWidgetProperty_HeaderColour, CColour( 50, 50, 50, 255 ) ) );
        Int32 x = area.left;
        Int32 sortColumn = widget.GetIntProperty( EWidgetProperty_SortColumn );
        for ( UInt32 c=0; c<(UInt32)widths.size(); ++c )
        {
            if ( c < (UInt32) columns.size() )
            {
                // a title that is wider than its column is cut off at the column
                CClipScope clipScope( commands, CRect( x, area.top, widths[ c ], rowHeight ) );
                Text( commands, font, columns[ c ], x + 2, area.top, textColour );
            }
            if ( (Int32) c == sortColumn && widths[ c ] > 16 )
            {
                // the arrow of the sort, at the right of the title
                CRect arrowBox( x + widths[ c ] - 14, area.top, 12, rowHeight );
                if ( widget.GetBoolProperty( EWidgetProperty_SortAscending ) )
                    UpArrow( commands, arrowBox, textColour );
                else
                    DownArrow( commands, arrowBox, textColour );
            }
            x += widths[ c ];
        }
        Int32 y = area.top + rowHeight;
        for ( Int32 row = scroll; row < (Int32) grid->GetRowCount() && y < area.Bottom(); ++row, y += rowHeight )
        {
            CRect rowRect( area.left, y, area.width, rowHeight );
            if ( grid->IsRowSelected( row ) )
                Fill( commands, rowRect, selectionColour );
            CGridView::TStringVector cells = grid->GetRowCells( (UInt32) row );
            x = area.left;
            for ( UInt32 c=0; c<(UInt32)widths.size(); ++c )
            {
                if ( c < (UInt32) cells.size() && !cells[ c ].IsNULLOrEmpty() )
                {
                    CClipScope clipScope( commands, CRect( x, y, widths[ c ], rowHeight ) );
                    Text( commands, font, cells[ c ], x + 2, y, textColour );
                }
                x += widths[ c ];
            }
            if ( row == hoverIndex )
                Stroke( commands, rowRect, hoverColour, 1 );
        }
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
            Text( commands, font, items[ row ], area.left + 8, y, menu->IsItemSelectable( row ) ? textColour : disabledColour );
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
                   widget.GetPropertyRef( EWidgetProperty_VTextAlign ), widget.GetBoolProperty( EWidgetProperty_WordWrap ), TextColourOf( widget ), commands );
        return;
    }
    if ( IsA( widget, "Editbox" ) )
    {
        PaintEditbox( widget, context, area, commands );
        return;
    }
    if ( IsA( widget, "Listbox" ) || IsA( widget, "Treeview" ) || IsA( widget, "Textbox" ) || IsA( widget, "GridView" ) || IsA( widget, "PopupMenu" ) )
    {
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) ) );
        PaintRows( widget, context, area, commands );
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
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), widget.GetPropertyRef( EWidgetProperty_Text ), CRect( area.left + 2, area.top, area.width - 2, area.height ),
                   "Left", "Center", false, TextColourOf( widget ), commands );
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
        PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), spinner->GetValueText(), area, "Center", "Center", false, colour, commands );
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
        Fill( commands, area, ColourOr( widget, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) ) );
        CTabControl::TStringVector titles = widget.GetStringListProperty( EWidgetProperty_Tabs );
        for ( UInt32 i=0; i<(UInt32)titles.size(); ++i )
        {
            CRect header = tabs->GetTabRect( i );
            bool active = (Int32) i == tabs->GetActiveTab();
            Fill( commands, header, ColourOr( widget, active ? EWidgetProperty_ActiveTabColour : EWidgetProperty_TabColour, BoxColour ) );
            Stroke( commands, header, BoxBorderColour, 1 );
            PaintText( context, widget.GetPropertyRef( EWidgetProperty_Font ), titles[ i ], header, "Center", "Center", false, TextColourOf( widget ), commands );
        }
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
                   "Left", "Center", false, TextColourOf( widget ), commands );
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
            Int32 textWidth = 0, textHeight = 0;
            context.MeasureText( font, caption, 0, textWidth, textHeight );
            const Int32 gapLeft = 8;
            const Int32 gapRight = gapLeft + 4 + textWidth + 4;
            Line( commands, 0, lineY, gapLeft, lineY, frame );
            if ( gapRight < right )
                Line( commands, gapRight, lineY, right, lineY, frame );
            PaintText( context, font, caption, CRect( gapLeft + 4, 0, textWidth + 1, captionHeight ), "Left", "Center", false, TextColourOf( widget ), commands );
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
                   "Left", "Center", false, titleColour, commands );
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

void
CWidgetPainter::PaintOverlay( CGUIContext& context, TCommandVector& commands )
{GUCEF_TRACE;

    // the tooltip of the widget the pointer rests on
    CWidget* hovered = context.GetHovered();
    if ( GUCEF_NULL != hovered && context.GetHoverDurationInMs() >= TooltipDelayInMs )
    {
        CString tooltip = hovered->GetPropertyRef( EWidgetProperty_Tooltip );
        if ( !tooltip.IsNULLOrEmpty() )
        {
            CString font = hovered->GetPropertyRef( EWidgetProperty_Font );
            Int32 width = 0, height = 0;
            context.MeasureText( font, tooltip, 0, width, height );
            if ( 0 == height )
                height = context.GetLineHeight( font );
            Int32 x = context.GetPointerX() + 12;
            Int32 y = context.GetPointerY() + 16;
            CRect box( x, y, width + 8, height + 4 );
            // kept inside the picture
            if ( box.Right() > (Int32) context.GetLogicalWidth() )
                box.left = (Int32) context.GetLogicalWidth() - box.width;
            if ( box.Bottom() > (Int32) context.GetLogicalHeight() )
                box.top = context.GetPointerY() - box.height - 4;
            Fill( commands, box, CColour( 250, 250, 210, 255 ) );
            Stroke( commands, box, CColour( 60, 60, 60, 255 ), 1 );
            Text( commands, font, tooltip, box.left + 4, box.top + 2, CColour( 0, 0, 0, 255 ) );
        }
    }

    // the cursor picture, not while a layer above takes the input (it has the pointer then)
    if ( context.IsCursorVisible() && !context.GetCursorImage().IsNULLOrEmpty() && !context.HasHigherLayerTakingInput() )
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
