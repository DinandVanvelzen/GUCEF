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

#include <algorithm>

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CGRIDEDITCONTROLLER_H
#include "gucefGUI_CGridEditController.h"
#define GUCEF_GUI_CGRIDEDITCONTROLLER_H
#endif /* GUCEF_GUI_CGRIDEDITCONTROLLER_H ? */

#include "gucefGUI_CGridViewPainter.h"

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

typedef CWidgetPainter::TCommandVector TCommands;

/* the look of the standard theme, as CWidgetPainter has it */
const CColour AccentColour( 74, 136, 232, 255 );
const CColour AccentTint( 74, 136, 232, 56 );
const CColour FocusColour( 200, 200, 90, 255 );
const CColour BorderColour( 120, 120, 128, 255 );
const CColour DisabledTextColour( 128, 128, 128, 255 );
const CColour NullCellTint( 128, 128, 128, 40 );
const CColour ScrollTrackColour( 255, 255, 255, 24 );
const CColour ScrollThumbColour( 255, 255, 255, 110 );
const CColour EditorFallbackColour( 30, 30, 34, 255 );
const CColour EditorSelectionColour( 74, 136, 232, 120 );
const CColour RefusedColour( 224, 64, 64, 255 );
const CColour RefusedFillColour( 86, 22, 26, 255 );
const CColour RefusedTextColour( 255, 214, 214, 255 );
const CColour ListFillColour( 36, 38, 44, 255 );

/** The dashes of the outline of copied cells */
const Int32 DashLength = 4;
const Int32 DashGap = 3;

/** The room of a scroll indicator */
const Int32 ScrollBarThickness = 6;

/** The side of the box of a boolean */
const Int32 CheckBoxSide = 10;

CColour
ColourOr( const CWidget& widget, EWidgetProperty property, const CColour& fallback )
{
    CColour colour;
    if ( !widget.TryGetColourProperty( property, colour ) )
        return fallback;
    return colour;
}

void
Fill( TCommands& commands, const CRect& rect, const CColour& colour )
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
Stroke( TCommands& commands, const CRect& rect, const CColour& colour, Int32 thickness )
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
Line( TCommands& commands, Int32 x0, Int32 y0, Int32 x1, Int32 y1, const CColour& colour )
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
Text( TCommands& commands, const CString& font, const CString& text, Int32 x, Int32 y, const CColour& colour )
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

/** Everything drawn while it lives is cut off at the rectangle */
class CClipScope
{
    public:

    CClipScope( TCommands& commands, const CRect& rect )
        : m_commands( commands )
    {
        CDrawSurface::TCommand command;
        command.type = CDrawSurface::COMMAND_PUSHCLIP;
        command.rect = rect;
        m_commands.push_back( command );
    }

    ~CClipScope()
    {
        CDrawSurface::TCommand command;
        command.type = CDrawSurface::COMMAND_POPCLIP;
        m_commands.push_back( command );
    }

    private:

    CClipScope( const CClipScope& src );
    CClipScope& operator=( const CClipScope& src );

    TCommands& m_commands;
};

void
UpArrow( TCommands& commands, const CRect& box, const CColour& colour )
{
    Int32 cx = box.left + box.width / 2;
    Int32 cy = box.top + box.height / 2;
    for ( Int32 i=0; i<4; ++i )
        Line( commands, cx - 3 + i, cy + 2 - i, cx + 3 - i, cy + 2 - i, colour );
}

void
DownArrow( TCommands& commands, const CRect& box, const CColour& colour )
{
    Int32 cx = box.left + box.width / 2;
    Int32 cy = box.top + box.height / 2;
    for ( Int32 i=0; i<4; ++i )
        Line( commands, cx - 3 + i, cy - 1 + i, cx + 3 - i, cy - 1 + i, colour );
}

/** The box of a boolean in the middle of a cell, with a tick when it is set */
void
CheckMark( TCommands& commands, const CRect& cell, bool on, const CColour& colour )
{
    const Int32 left = cell.left + ( cell.width - CheckBoxSide ) / 2;
    const Int32 top = cell.top + ( cell.height - CheckBoxSide ) / 2;
    Stroke( commands, CRect( left, top, CheckBoxSide, CheckBoxSide ), colour, 1 );
    if ( on )
    {
        Line( commands, left + 2, top + 5, left + 4, top + 7, colour );
        Line( commands, left + 4, top + 7, left + 8, top + 3, colour );
        Line( commands, left + 2, top + 4, left + 4, top + 6, colour );
        Line( commands, left + 4, top + 6, left + 8, top + 2, colour );
    }
}

/**
 *  One painting of a grid. Reads the geometry and the cells in view from the grid, in the coordinates of the grid
 *  (the area is at the origin), and appends the commands. The order of the commands for a grid that uses none of the
 *  optional features is the order it always had.
 */
class CGridPaint
{
    public:

    CGridPaint( const CGridView& grid, CGUIContext& context, const CRect& area, TCommands& commands )
        : m_grid( grid )
        , m_context( context )
        , m_area( area )
        , m_commands( commands )
        , m_layout( grid.GetColumnLayout() )
        , m_font( grid.GetPropertyRef( EWidgetProperty_Font ) )
        , m_rowHeight( grid.GetRowHeight() )
        , m_textColour( ColourOr( grid, EWidgetProperty_TextColour, CColour( 255, 255, 255, 255 ) ) )
        , m_selectionColour( ColourOr( grid, EWidgetProperty_SelectionColour, CColour( 0, 0, 0, 0 ) ) )
        , m_hoverColour( ColourOr( grid, EWidgetProperty_HoverColour, CColour( 0, 0, 0, 0 ) ) )
        , m_headerColour( ColourOr( grid, EWidgetProperty_HeaderColour, CColour( 50, 50, 50, 255 ) ) )
        , m_emphasisColour( ColourOr( grid, EWidgetProperty_EmphasisColour, CColour( 150, 198, 255, 255 ) ) )
        , m_gridColour( ColourOr( grid, EWidgetProperty_GridColour, CColour( 70, 70, 76, 255 ) ) )
        , m_hasZebra( grid.TryGetColourProperty( EWidgetProperty_ZebraColour, m_zebraColour ) )
        , m_hoverIndex( grid.GetIntProperty( EWidgetProperty_HoverIndex ) )
        , m_ellipsis( "Ellipsis" == grid.GetPropertyRef( EWidgetProperty_TextOverflow ) )
        , m_cellMode( grid.IsCellMode() )
        , m_rowCount( (Int32) grid.GetRowCount() )
        , m_frozenRows( grid.GetFrozenRowCount() )
        , m_firstScrolledRow( grid.GetFirstScrolledRow() )
        , m_footerTop( grid.GetFooterTop() )
        , m_rowHeaderWidth( m_layout.GetRowHeaderWidth() )
        , m_emphasizedRows( grid.GetEmphasizedRows() )
        , m_selectedRows()
    {
        const CString& lines = grid.GetPropertyRef( EWidgetProperty_GridLines );
        m_horizontalLines = "Horizontal" == lines || "Both" == lines;
        m_verticalLines = "Vertical" == lines || "Both" == lines;
        if ( !m_cellMode )
            m_selectedRows = grid.GetSelectedRows();
        m_dataBottom = m_footerTop >= 0 ? m_area.top + m_footerTop : m_area.Bottom();
        m_scrollRowsTop = m_area.top + m_rowHeight + m_frozenRows * m_rowHeight;

        // the columns drawn: all of them, or the frozen ones and the ones in view when the columns scroll
        const Int32 count = (Int32) m_layout.GetPositionCount();
        if ( m_layout.GetFrozenCount() > 0 || m_layout.GetMaxScrollX() > 0 )
        {
            for ( Int32 p=0; p<m_layout.GetFrozenCount(); ++p )
                m_positions.push_back( p );
            Int32 first = 0, last = -1;
            m_layout.GetVisibleScrolledPositions( first, last );
            for ( Int32 p=first; p<=last; ++p )
                m_positions.push_back( p );
        }
        else
        {
            for ( Int32 p=0; p<count; ++p )
                m_positions.push_back( p );
        }
        for ( Int32 p=0; p<count; ++p )
            m_alignments.push_back( grid.GetColumnAlignment( (UInt32) m_layout.GetColumn( p ) ) );
        m_scrolled = m_layout.GetFrozenCount() > 0 || m_layout.GetScrollX() > 0;
        m_rowsClipped = m_frozenRows > 0 || m_footerTop >= 0;
    }

    void Paint( void )
    {
        m_grid.PrepareVisibleCells();
        PaintHeader();
        PaintRows();
        PaintFooter();
        PaintLines();
        PaintDecorations();
        PaintCopyOutline();
        PaintEditor();
    }

    private:

    /** The room the columns that scroll are seen in, and the room the rows that scroll are seen in */
    CRect ColumnRegion( Int32 position ) const
    {
        if ( m_layout.IsFrozen( position ) )
            return CRect( m_area.left + m_rowHeaderWidth, m_area.top, m_layout.GetFrozenWidth(), m_area.height );
        const Int32 left = m_area.left + m_rowHeaderWidth + m_layout.GetFrozenWidth();
        return CRect( left, m_area.top, m_area.Right() - left, m_area.height );
    }

    CRect RowRegion( Int32 row ) const
    {
        if ( row < m_frozenRows )
            return CRect( m_area.left, m_area.top + m_rowHeight, m_area.width, m_frozenRows * m_rowHeight );
        return CRect( m_area.left, m_scrollRowsTop, m_area.width, m_dataBottom - m_scrollRowsTop );
    }

    /** The rectangle a cell is cut off at: its own, and the area its columns and rows scroll in when they scroll */
    CRect CellClip( const CRect& cell, Int32 position, Int32 row ) const
    {
        CRect clip = cell;
        if ( m_scrolled && !m_layout.IsFrozen( position ) )
            clip = clip.Intersection( ColumnRegion( position ) );
        if ( m_rowsClipped && row >= m_frozenRows )
            clip = clip.Intersection( RowRegion( row ) );
        return clip;
    }

    Int32 RowY( Int32 slot ) const
    {
        return m_area.top + m_rowHeight + slot * m_rowHeight;
    }

    /** The row drawn in a slot below the header, the frozen rows first */
    Int32 RowOfSlot( Int32 slot ) const
    {
        return slot < m_frozenRows ? slot : m_firstScrolledRow + ( slot - m_frozenRows );
    }

    bool IsSelectedRow( Int32 row ) const
    {
        return std::find( m_selectedRows.begin(), m_selectedRows.end(), row ) != m_selectedRows.end();
    }

    void PaintTitle( Int32 position )
    {
        const Int32 column = m_layout.GetColumn( position );
        const Int32 x = m_area.left + m_layout.GetScreenLeft( position );
        const Int32 width = m_layout.GetWidth( position );
        const CRect titleCell( x, m_area.top, width, m_rowHeight );
        const Int32 sortColumn = m_grid.GetIntProperty( EWidgetProperty_SortColumn );
        const bool sorted = column == sortColumn && width > (Int32) CGridView::SORT_ARROW_ROOM;
        const CString& alignment = m_alignments[ position ];

        // the column the rows are sorted by is marked as a whole: a tint of the accent over its title, the arrow right beside the title
        if ( sorted )
            Fill( m_commands, titleCell, AccentTint );
        else
        if ( m_cellMode && m_grid.GetSelection().ContainsWholeColumn( position ) )
            Fill( m_commands, titleCell, AccentTint );
        CString title = m_grid.GetColumnTitle( (UInt32) column );
        const Int32 room = width - (Int32) CGridView::CELL_PADDING_LEFT - (Int32) CGridView::CELL_PADDING_RIGHT - ( sorted ? (Int32) CGridView::SORT_ARROW_ROOM : 0 );
        Int32 titleWidth = 0, titleHeight = 0;
        if ( m_ellipsis )
        {
            bool cut = false;
            title = CWidgetPainter::FitText( m_context, m_font, title, room, cut );
        }
        if ( sorted || "Left" != alignment )
            m_context.MeasureText( m_font, title, 0, titleWidth, titleHeight );

        // a title that is wider than its column is cut off at the column
        CClipScope clipScope( m_commands, m_scrolled && !m_layout.IsFrozen( position ) ? titleCell.Intersection( ColumnRegion( position ) ) : titleCell );
        const Int32 padLeft = CGridView::CELL_PADDING_LEFT;
        const Int32 padRight = CGridView::CELL_PADDING_RIGHT;
        Int32 titleX = x + padLeft;
        Int32 arrowX = 0;
        if ( "Right" == alignment )
        {
            titleX = CWidgetPainter::AlignText( x, width, titleWidth, "Right", padLeft, padRight );
            arrowX = titleX - (Int32) CGridView::SORT_ARROW_ROOM + 2;
        }
        else
        {
            if ( "Center" == alignment )
                titleX = CWidgetPainter::AlignText( x, width - ( sorted ? (Int32) CGridView::SORT_ARROW_ROOM : 0 ), titleWidth, "Center", padLeft, padRight );
            arrowX = titleX + titleWidth + 2;
        }
        Text( m_commands, m_font, title, titleX, m_area.top, m_textColour );
        if ( sorted )
        {
            CRect arrowBox( arrowX, m_area.top, 12, m_rowHeight );
            if ( m_grid.GetBoolProperty( EWidgetProperty_SortAscending ) )
                UpArrow( m_commands, arrowBox, m_textColour );
            else
                DownArrow( m_commands, arrowBox, m_textColour );
        }
    }

    void PaintHeader( void )
    {
        Fill( m_commands, CRect( m_area.left, m_area.top, m_area.width, m_rowHeight ), m_headerColour );
        for ( size_t i=0; i<m_positions.size(); ++i )
            PaintTitle( m_positions[ i ] );
        if ( m_rowHeaderWidth > 0 )
        {
            // the corner above the row header selects everything
            const CRect corner( m_area.left, m_area.top, m_rowHeaderWidth, m_rowHeight );
            Fill( m_commands, corner, m_headerColour );
            if ( m_cellMode && m_grid.GetSelection().GetRangeCount() > 0 && m_grid.GetSelection().GetRange( 0 ).wholeRows && m_grid.GetSelection().GetRange( 0 ).wholeColumns )
                Fill( m_commands, corner, AccentTint );
            Line( m_commands, corner.Right() - 1, corner.top, corner.Right() - 1, corner.Bottom(), BorderColour );
        }
    }

    /** The content of a cell of a row of cells that were split already (the string list) or asked of the model */
    bool ContentOf( Int32 row, Int32 column, const CGridView::TStringVector& cells, SCellContent& content ) const
    {
        if ( m_grid.HasModel() )
            return m_grid.GetCellContent( row, column, content );
        content.kind = SCellContent::KIND_TEXT;
        content.isNumber = false;
        if ( column < (Int32) cells.size() )
            content.text = cells[ column ];
        else
            content.text.Clear();
        return column < (Int32) cells.size();
    }

    void PaintCell( Int32 row, Int32 position, Int32 y, bool emphasized, const CGridView::TStringVector& cells )
    {
        const Int32 column = m_layout.GetColumn( position );
        const Int32 x = m_area.left + m_layout.GetScreenLeft( position );
        const Int32 width = m_layout.GetWidth( position );
        SCellContent content;
        ContentOf( row, column, cells, content );
        const CRect cell( x, y, width, m_rowHeight );

        if ( SCellContent::KIND_NULL == content.kind )
        {
            // a null is an empty cell of its own: tinted, with the null text of the column when it has one
            Fill( m_commands, CellClip( cell, position, row ), NullCellTint );
        }
        if ( SCellContent::KIND_CHECK_ON == content.kind || SCellContent::KIND_CHECK_OFF == content.kind )
        {
            CClipScope clipScope( m_commands, CellClip( cell, position, row ) );
            CheckMark( m_commands, cell, SCellContent::KIND_CHECK_ON == content.kind, emphasized ? m_emphasisColour : m_textColour );
            return;
        }
        if ( content.text.IsNULLOrEmpty() )
            return;

        const Int32 padLeft = CGridView::CELL_PADDING_LEFT;
        const Int32 padRight = CGridView::CELL_PADDING_RIGHT;
        const Int32 indent = emphasized && 0 == position ? (Int32) CGridView::EMPHASIS_INDENT : 0;
        CString text = content.text;
        if ( m_ellipsis )
        {
            bool cut = false;
            text = CWidgetPainter::FitText( m_context, m_font, text, width - padLeft - padRight - indent, cut );
        }
        Int32 textX = x + padLeft + indent;
        const CString& alignment = m_alignments[ position ];
        if ( "Left" != alignment )
        {
            Int32 textWidth = 0, textHeight = 0;
            m_context.MeasureText( m_font, text, 0, textWidth, textHeight );
            textX = CWidgetPainter::AlignText( x + indent, width - indent, textWidth, alignment, padLeft, padRight );
        }
        CClipScope clipScope( m_commands, CellClip( cell, position, row ) );
        const CColour& colour = SCellContent::KIND_NULL == content.kind ? DisabledTextColour : ( emphasized ? m_emphasisColour : m_textColour );
        Text( m_commands, m_font, text, textX, y, colour );
    }

    /** The cells of a row in a row that are selected, as runs of neighbours: one fill for each */
    void PaintSelectedCells( Int32 row, Int32 y )
    {
        const CGridSelection& selection = m_grid.GetSelection();
        if ( selection.IsEmpty() )
            return;
        const Int32 cursorRow = m_grid.GetCursorRow();
        const Int32 cursorPosition = m_layout.FindPosition( m_grid.GetCursorColumn() );
        size_t i = 0;
        while ( i < m_positions.size() )
        {
            const Int32 position = m_positions[ i ];
            // a block of one cell is the cursor and has no fill: the cursor cell is the one that is not tinted
            const bool selected = selection.Contains( row, position ) && !( row == cursorRow && position == cursorPosition );
            if ( !selected )
            {
                ++i;
                continue;
            }
            size_t j = i;
            while ( j + 1 < m_positions.size() && m_positions[ j + 1 ] == m_positions[ j ] + 1 && m_layout.IsFrozen( m_positions[ j + 1 ] ) == m_layout.IsFrozen( m_positions[ j ] ) &&
                    selection.Contains( row, m_positions[ j + 1 ] ) && !( row == cursorRow && m_positions[ j + 1 ] == cursorPosition ) )
                ++j;
            const Int32 left = m_area.left + m_layout.GetScreenLeft( m_positions[ i ] );
            const Int32 right = m_area.left + m_layout.GetScreenLeft( m_positions[ j ] ) + m_layout.GetWidth( m_positions[ j ] );
            CRect run( left, y, right - left, m_rowHeight );
            if ( m_scrolled && !m_layout.IsFrozen( m_positions[ i ] ) )
                run = run.Intersection( ColumnRegion( m_positions[ i ] ) );
            if ( m_rowsClipped && row >= m_frozenRows )
                run = run.Intersection( RowRegion( row ) );
            Fill( m_commands, run, m_selectionColour );
            i = j + 1;
        }
    }

    void PaintRowHeaderCell( Int32 row, Int32 y )
    {
        const CRect cell( m_area.left, y, m_rowHeaderWidth, m_rowHeight );
        Fill( m_commands, cell, m_headerColour );
        const bool selected = m_cellMode ? ( m_grid.GetSelection().ContainsWholeRow( row ) ) : IsSelectedRow( row );
        const bool cursor = m_cellMode && row == m_grid.GetCursorRow();
        if ( selected )
            Fill( m_commands, cell, AccentTint );
        else
        if ( cursor )
            Fill( m_commands, cell, CColour( 74, 136, 232, 28 ) );
        const CString title = m_grid.GetRowHeaderText( row );
        Int32 textWidth = 0, textHeight = 0;
        m_context.MeasureText( m_font, title, 0, textWidth, textHeight );
        CClipScope clipScope( m_commands, cell );
        Text( m_commands, m_font, title, CWidgetPainter::AlignText( cell.left, cell.width, textWidth, "Right", CGridView::CELL_PADDING_LEFT + 2, CGridView::CELL_PADDING_RIGHT - 2 ), y, m_textColour );
        Line( m_commands, cell.Right() - 1, cell.top, cell.Right() - 1, cell.Bottom(), BorderColour );
    }

    void PaintRow( Int32 row, Int32 y )
    {
        const CRect rowRect( m_area.left, y, m_area.width, m_rowHeight );
        if ( m_hasZebra && 1 == ( row & 1 ) )
            Fill( m_commands, m_rowsClipped && row >= m_frozenRows ? rowRect.Intersection( RowRegion( row ) ) : rowRect, m_zebraColour );

        // a row that needs the player stands out: a tint of the accent over the whole row, a bar of the accent at its left edge and the text in the colour of the emphasis
        const bool emphasized = std::binary_search( m_emphasizedRows.begin(), m_emphasizedRows.end(), row ) || m_grid.IsRowEmphasizedByModel( row );
        if ( emphasized )
        {
            Fill( m_commands, rowRect, AccentTint );
            Fill( m_commands, CRect( m_area.left, y, 4, m_rowHeight ), AccentColour );
        }
        if ( m_cellMode )
            PaintSelectedCells( row, y );
        else
        if ( IsSelectedRow( row ) )
            Fill( m_commands, rowRect, m_selectionColour );
        if ( m_rowHeaderWidth > 0 )
            PaintRowHeaderCell( row, y );

        CGridView::TStringVector cells;
        if ( !m_grid.HasModel() )
            cells = m_grid.GetRowCells( (UInt32) row );
        for ( size_t i=0; i<m_positions.size(); ++i )
            PaintCell( row, m_positions[ i ], y, emphasized, cells );
        if ( row == m_hoverIndex )
            Stroke( m_commands, rowRect, m_hoverColour, 1 );
    }

    void PaintRows( void )
    {
        // the rows that scroll are cut off above the footer and below the frozen rows
        for ( Int32 slot=0; ; ++slot )
        {
            const Int32 row = RowOfSlot( slot );
            const Int32 y = RowY( slot );
            if ( row >= m_rowCount || y >= m_dataBottom )
                break;
            PaintRow( row, y );
        }
    }

    void PaintFooter( void )
    {
        if ( m_footerTop < 0 )
            return;
        const Int32 y = m_area.top + m_footerTop;
        const CRect strip( m_area.left, y, m_area.width, m_rowHeight );
        Fill( m_commands, strip, m_headerColour );
        Line( m_commands, strip.left, y, strip.Right(), y, BorderColour );
        for ( size_t i=0; i<m_positions.size(); ++i )
        {
            const Int32 position = m_positions[ i ];
            SCellContent content;
            if ( !m_grid.GetFooterContent( (UInt32) m_layout.GetColumn( position ), content ) || content.text.IsNULLOrEmpty() )
                continue;
            const Int32 x = m_area.left + m_layout.GetScreenLeft( position );
            const Int32 width = m_layout.GetWidth( position );
            Int32 textWidth = 0, textHeight = 0;
            m_context.MeasureText( m_font, content.text, 0, textWidth, textHeight );
            CClipScope clipScope( m_commands, m_scrolled && !m_layout.IsFrozen( position ) ? CRect( x, y, width, m_rowHeight ).Intersection( ColumnRegion( position ) ) : CRect( x, y, width, m_rowHeight ) );
            Text( m_commands, m_font, content.text, CWidgetPainter::AlignText( x, width, textWidth, m_alignments[ position ], CGridView::CELL_PADDING_LEFT, CGridView::CELL_PADDING_RIGHT ), y, m_textColour );
        }
        if ( m_rowHeaderWidth > 0 )
            Line( m_commands, m_area.left + m_rowHeaderWidth - 1, y, m_area.left + m_rowHeaderWidth - 1, strip.Bottom(), BorderColour );
    }

    void PaintLines( void )
    {
        if ( !m_horizontalLines && !m_verticalLines )
            return;
        const Int32 left = m_area.left + m_rowHeaderWidth;
        const Int32 top = m_area.top + m_rowHeight;
        Int32 bottom = top;
        for ( Int32 slot=0; ; ++slot )
        {
            const Int32 row = RowOfSlot( slot );
            const Int32 y = RowY( slot );
            if ( row >= m_rowCount || y >= m_dataBottom )
                break;
            bottom = y + m_rowHeight < m_dataBottom ? y + m_rowHeight : m_dataBottom;
            if ( m_horizontalLines )
                Line( m_commands, left, bottom - 1, m_area.Right(), bottom - 1, m_gridColour );
        }
        if ( !m_verticalLines || bottom <= top )
            return;
        for ( size_t i=0; i<m_positions.size(); ++i )
        {
            const Int32 position = m_positions[ i ];
            const Int32 edge = m_area.left + m_layout.GetScreenLeft( position ) + m_layout.GetWidth( position ) - 1;
            if ( !m_layout.IsFrozen( position ) && ( edge < left + m_layout.GetFrozenWidth() || edge >= m_area.Right() ) )
                continue;
            Line( m_commands, edge, top, edge, bottom, m_gridColour );
        }
    }

    void PaintScrollBars( void )
    {
        const Int32 top = m_area.top + m_rowHeight;
        const Int32 height = m_dataBottom - top;
        if ( m_rowCount > 0 && height > 0 )
        {
            const Int32 slots = m_grid.GetVisibleRowCount() - m_frozenRows;
            const Int32 scrollable = m_rowCount - m_frozenRows;
            if ( scrollable > slots && slots > 0 )
            {
                const CRect track( m_area.Right() - ScrollBarThickness, top, ScrollBarThickness, height );
                Fill( m_commands, track, ScrollTrackColour );
                Int32 thumb = (Int32) ( (Int64) height * slots / scrollable );
                if ( thumb < 12 ) thumb = 12;
                if ( thumb > height ) thumb = height;
                const Int32 max = m_grid.GetMaxScrollRow() - m_frozenRows;
                const Int32 position = max > 0 ? (Int32) ( (Int64) ( height - thumb ) * ( m_firstScrolledRow - m_frozenRows ) / max ) : 0;
                Fill( m_commands, CRect( track.left, top + position, ScrollBarThickness, thumb ), ScrollThumbColour );
            }
        }
        const Int32 maxX = m_layout.GetMaxScrollX();
        if ( maxX > 0 )
        {
            const Int32 left = m_area.left + m_rowHeaderWidth + m_layout.GetFrozenWidth();
            const Int32 width = m_area.Right() - left;
            if ( width > 0 )
            {
                const CRect track( left, m_dataBottom - ScrollBarThickness, width, ScrollBarThickness );
                Fill( m_commands, track, ScrollTrackColour );
                Int32 thumb = (Int32) ( (Int64) width * m_layout.GetScrollViewWidth() / ( m_layout.GetContentWidth() - m_layout.GetFrozenWidth() ) );
                if ( thumb < 12 ) thumb = 12;
                if ( thumb > width ) thumb = width;
                const Int32 position = (Int32) ( (Int64) ( width - thumb ) * m_layout.GetScrollX() / maxX );
                Fill( m_commands, CRect( left + position, track.top, thumb, ScrollBarThickness ), ScrollThumbColour );
            }
        }
    }

    void PaintDecorations( void )
    {
        // where the frozen columns and rows end
        if ( m_layout.GetFrozenCount() > 0 )
        {
            const Int32 x = m_area.left + m_rowHeaderWidth + m_layout.GetFrozenWidth();
            Fill( m_commands, CRect( x - 1, m_area.top, 2, m_dataBottom - m_area.top ), BorderColour );
        }
        if ( m_frozenRows > 0 )
            Fill( m_commands, CRect( m_area.left, m_scrollRowsTop - 1, m_area.width, 2 ), BorderColour );

        // the cursor
        if ( m_cellMode && m_grid.GetCursorRow() >= 0 && m_grid.GetCursorColumn() >= 0 )
        {
            CRect cell;
            if ( m_grid.GetCellRect( m_grid.GetCursorRow(), m_grid.GetCursorColumn(), cell ) )
            {
                cell.left += m_area.left;
                cell.top += m_area.top;
                Stroke( m_commands, cell, AccentColour, 2 );
            }
        }

        // where a column that is dragged would go
        const Int32 drop = m_grid.GetColumnDropPosition();
        if ( drop >= 0 )
        {
            const Int32 count = (Int32) m_layout.GetPositionCount();
            Int32 x = drop >= count ? m_area.left + m_layout.GetScreenLeft( count - 1 ) + m_layout.GetWidth( count - 1 ) : m_area.left + m_layout.GetScreenLeft( drop );
            Fill( m_commands, CRect( x - 1, m_area.top, 3, m_dataBottom - m_area.top ), FocusColour );
        }
        if ( m_grid.GetBoolProperty( EWidgetProperty_ScrollBars ) )
            PaintScrollBars();
    }

    /** A dashed line, horizontal or vertical, cut to the room the lines are seen in */
    void DashedLine( Int32 x0, Int32 y0, Int32 x1, Int32 y1, const CRect& room, const CColour& colour )
    {
        if ( y0 == y1 )
        {
            if ( y0 < room.top || y0 >= room.Bottom() )
                return;
            Int32 from = x0 < room.left - DashLength ? room.left - DashLength : x0;
            const Int32 to = x1 > room.Right() + DashLength ? room.Right() + DashLength : x1;
            for ( Int32 x = from; x < to; x += DashLength + DashGap )
                Line( m_commands, x, y0, x + DashLength - 1 < to ? x + DashLength - 1 : to, y0, colour );
        }
        else
        {
            if ( x0 < room.left || x0 >= room.Right() )
                return;
            Int32 from = y0 < room.top - DashLength ? room.top - DashLength : y0;
            const Int32 to = y1 > room.Bottom() + DashLength ? room.Bottom() + DashLength : y1;
            for ( Int32 y = from; y < to; y += DashLength + DashGap )
                Line( m_commands, x0, y, x0, y + DashLength - 1 < to ? y + DashLength - 1 : to, colour );
        }
    }

    /** The y of the top of a row, also for a row that is scrolled out of view */
    Int32 RowTopAnywhere( Int32 row ) const
    {
        if ( row < m_frozenRows )
            return m_area.top + m_rowHeight + row * m_rowHeight;
        return m_scrollRowsTop + ( row - m_firstScrolledRow ) * m_rowHeight;
    }

    /** The cells that were copied: a dashed outline round every range, as far as it is in view */
    void PaintCopyOutline( void )
    {
        const CGridSelection* outline = m_grid.GetCopyOutline();
        if ( GUCEF_NULL == outline )
            return;
        const Int32 left = m_area.left + m_rowHeaderWidth;
        const Int32 top = m_area.top + m_rowHeight;
        const CRect room( left, top, m_area.Right() - left, m_dataBottom - top );
        if ( room.IsEmpty() )
            return;
        const Int32 positionCount = (Int32) m_layout.GetPositionCount();
        for ( UInt32 i=0; i<outline->GetRangeCount(); ++i )
        {
            const CGridSelection::SRange& range = outline->GetRange( i );
            if ( range.firstColumn >= positionCount )
                continue;
            const Int32 lastPosition = range.lastColumn < positionCount ? range.lastColumn : positionCount - 1;
            const Int32 x0 = m_area.left + m_layout.GetScreenLeft( range.firstColumn );
            const Int32 x1 = m_area.left + m_layout.GetScreenLeft( lastPosition ) + m_layout.GetWidth( lastPosition ) - 1;
            const Int32 y0 = RowTopAnywhere( range.firstRow );
            const Int32 y1 = RowTopAnywhere( range.lastRow ) + m_rowHeight - 1;
            DashedLine( x0, y0, x1, y0, room, FocusColour );
            DashedLine( x0, y1, x1, y1, room, FocusColour );
            DashedLine( x0, y0, x0, y1, room, FocusColour );
            DashedLine( x1, y0, x1, y1, room, FocusColour );
        }
    }

    /** The editor of a cell over the cell, the list of choices under it, and the reason a value was refused */
    void PaintEditor( void )
    {
        const CGridEditController* controller = m_grid.GetEditController();
        if ( GUCEF_NULL == controller || !controller->IsEditing() )
            return;
        CGridEditController::SEditorLayout layout;
        if ( !controller->GetEditorLayout( layout ) )
            return;
        const CGridCellEditor& editor = controller->GetEditor();
        const CRect cell( m_area.left + layout.cell.left, m_area.top + layout.cell.top, layout.cell.width, layout.cell.height );
        const CRect visible( m_area.left + layout.clip.left, m_area.top + layout.clip.top, layout.clip.width, layout.clip.height );
        CColour field = ColourOr( m_grid, EWidgetProperty_BackgroundColour, EditorFallbackColour );
        field.a = 255;
        const bool refused = editor.HasError();
        {
            CClipScope clipScope( m_commands, visible );
            Fill( m_commands, cell, field );
            if ( editor.HasSelection() )
                Fill( m_commands, CRect( m_area.left + layout.selectionLeft, cell.top + 1, layout.selectionRight - layout.selectionLeft, cell.height - 2 ), EditorSelectionColour );
            Text( m_commands, m_font, editor.GetText(), m_area.left + layout.textLeft, cell.top, m_textColour );
            if ( CGridCellEditor::KIND_CHOICE == editor.GetKind() )
                DownArrow( m_commands, CRect( cell.Right() - 16, cell.top, 12, cell.height ), m_textColour );
            else
            if ( !editor.HasSelection() )
                Line( m_commands, m_area.left + layout.caretX, cell.top + 2, m_area.left + layout.caretX, cell.Bottom() - 3, m_textColour );
            Stroke( m_commands, cell, refused ? RefusedColour : AccentColour, 2 );
        }
        if ( layout.hasList )
        {
            const CRect list( m_area.left + layout.list.left, m_area.top + layout.list.top, layout.list.width, layout.list.height );
            Fill( m_commands, list, ListFillColour );
            const Int32 current = editor.GetChoiceIndex();
            for ( Int32 i=0; i<layout.itemCount; ++i )
            {
                const Int32 index = layout.firstItem + i;
                const CRect item( list.left, list.top + i * layout.itemHeight, list.width, layout.itemHeight );
                if ( index == current )
                    Fill( m_commands, item, EditorSelectionColour );
                CClipScope itemClip( m_commands, item );
                Text( m_commands, m_font, editor.GetChoices()[ index ], item.left + CGridEditController::EDITOR_PADDING, item.top, m_textColour );
            }
            Stroke( m_commands, list, AccentColour, 1 );
        }
        if ( layout.hasMessage )
        {
            const CRect message( m_area.left + layout.message.left, m_area.top + layout.message.top, layout.message.width, layout.message.height );
            Fill( m_commands, message, RefusedFillColour );
            CClipScope messageClip( m_commands, message );
            Text( m_commands, m_font, editor.GetError(), message.left + 6, message.top, RefusedTextColour );
            Stroke( m_commands, message, RefusedColour, 1 );
        }
    }

    private:

    const CGridView& m_grid;
    CGUIContext& m_context;
    const CRect& m_area;
    TCommands& m_commands;
    const CGridColumnLayout& m_layout;
    const CString& m_font;
    const Int32 m_rowHeight;
    const CColour m_textColour;
    const CColour m_selectionColour;
    const CColour m_hoverColour;
    const CColour m_headerColour;
    const CColour m_emphasisColour;
    const CColour m_gridColour;
    CColour m_zebraColour;
    const bool m_hasZebra;
    const Int32 m_hoverIndex;
    const bool m_ellipsis;
    const bool m_cellMode;
    const Int32 m_rowCount;
    const Int32 m_frozenRows;
    const Int32 m_firstScrolledRow;
    const Int32 m_footerTop;
    const Int32 m_rowHeaderWidth;
    const CGridView::TInt32Vector m_emphasizedRows;
    CGridView::TInt32Vector m_selectedRows;
    bool m_horizontalLines;
    bool m_verticalLines;
    bool m_scrolled;
    bool m_rowsClipped;
    Int32 m_dataBottom;
    Int32 m_scrollRowsTop;
    GUCEF::vector< Int32 > m_positions;
    GUCEF::vector< CString > m_alignments;

    CGridPaint( const CGridPaint& src );
    CGridPaint& operator=( const CGridPaint& src );
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CGridViewPainter::Paint( const CGridView& grid, CGUIContext& context, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    CGridPaint paint( grid, context, area, commands );
    paint.Paint();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
