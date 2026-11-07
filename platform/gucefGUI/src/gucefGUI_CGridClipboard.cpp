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

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CTABLECELLPARSER_H
#include "gucefGUI_CTableCellParser.h"
#define GUCEF_GUI_CTABLECELLPARSER_H
#endif /* GUCEF_GUI_CTABLECELLPARSER_H ? */

#ifndef GUCEF_GUI_CTABULARTEXT_H
#include "gucefGUI_CTabularText.h"
#define GUCEF_GUI_CTABULARTEXT_H
#endif /* GUCEF_GUI_CTABULARTEXT_H ? */

#include "gucefGUI_CGridClipboard.h"

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

/** Keeps the changes of many cells together: the model's edit group and the update of the grid, closed in the reverse order */
class CEditScope
{
    public:

    CEditScope( CGridView& grid, CITableViewModel* model )
        : m_grid( grid ), m_model( model )
    {
        m_grid.BeginUpdate();
        if ( GUCEF_NULL != m_model )
            m_model->BeginEditGroup();
    }

    ~CEditScope()
    {
        if ( GUCEF_NULL != m_model )
            m_model->EndEditGroup();
        m_grid.EndUpdate();
    }

    private:

    CEditScope( const CEditScope& src );
    CEditScope& operator=( const CEditScope& src );

    CGridView& m_grid;
    CITableViewModel* m_model;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridClipboard::CGridClipboard( CGridView& grid )
    : m_grid( grid )
    , m_outline()
    , m_hasOutline( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridClipboard::~CGridClipboard()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CGridSelection*
CGridClipboard::GetOutline( void ) const
{GUCEF_TRACE;

    return m_hasOutline ? &m_outline : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::ClearOutline( void )
{GUCEF_TRACE;

    if ( !m_hasOutline )
        return;
    m_hasOutline = false;
    m_outline.Clear();
    m_grid.Touch( EWidgetProperty_Rows );
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::OnModelChanged( void )
{GUCEF_TRACE;

    ClearOutline();
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Resolve( TBlockVector& blocks ) const
{GUCEF_TRACE;

    blocks.clear();
    const Int32 rows = (Int32) m_grid.GetRowCount();
    const CGridColumnLayout& layout = m_grid.GetColumnLayout();
    const Int32 positions = (Int32) layout.GetPositionCount();
    if ( rows <= 0 || positions <= 0 )
        return false;
    const CGridSelection& selection = m_grid.GetSelection();
    for ( UInt32 i=0; i<selection.GetRangeCount(); ++i )
    {
        const CGridSelection::SRange& range = selection.GetRange( i );
        SBlock block;
        block.firstRow = range.wholeColumns ? 0 : range.firstRow;
        block.lastRow = range.wholeColumns ? rows - 1 : range.lastRow;
        block.firstPosition = range.wholeRows ? 0 : range.firstColumn;
        block.lastPosition = range.wholeRows ? positions - 1 : range.lastColumn;
        if ( block.firstRow < 0 ) block.firstRow = 0;
        if ( block.firstPosition < 0 ) block.firstPosition = 0;
        if ( block.lastRow >= rows ) block.lastRow = rows - 1;
        if ( block.lastPosition >= positions ) block.lastPosition = positions - 1;
        if ( block.firstRow <= block.lastRow && block.firstPosition <= block.lastPosition )
            blocks.push_back( block );
    }
    if ( blocks.empty() && m_grid.GetCursorRow() >= 0 && m_grid.GetCursorColumn() >= 0 )
    {
        const Int32 position = layout.FindPosition( m_grid.GetCursorColumn() );
        if ( position >= 0 && m_grid.GetCursorRow() < rows )
        {
            SBlock block;
            block.firstRow = block.lastRow = m_grid.GetCursorRow();
            block.firstPosition = block.lastPosition = position;
            blocks.push_back( block );
        }
    }
    return !blocks.empty();
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::OrderForCopy( TBlockVector& blocks, bool& sideBySide ) const
{GUCEF_TRACE;

    sideBySide = true;
    if ( blocks.size() <= 1 )
        return true;
    bool sameRows = true, sameColumns = true;
    for ( size_t i=1; i<blocks.size(); ++i )
    {
        if ( blocks[ i ].firstRow != blocks[ 0 ].firstRow || blocks[ i ].lastRow != blocks[ 0 ].lastRow )
            sameRows = false;
        if ( blocks[ i ].firstPosition != blocks[ 0 ].firstPosition || blocks[ i ].lastPosition != blocks[ 0 ].lastPosition )
            sameColumns = false;
    }
    if ( sameRows )
    {
        // side by side, in the order of the columns
        for ( size_t i=1; i<blocks.size(); ++i )
        {
            size_t j = i;
            while ( j > 0 && blocks[ j ].firstPosition < blocks[ j - 1 ].firstPosition )
            {
                std::swap( blocks[ j ], blocks[ j - 1 ] );
                --j;
            }
        }
        sideBySide = true;
        return true;
    }
    if ( sameColumns )
    {
        for ( size_t i=1; i<blocks.size(); ++i )
        {
            size_t j = i;
            while ( j > 0 && blocks[ j ].firstRow < blocks[ j - 1 ].firstRow )
            {
                std::swap( blocks[ j ], blocks[ j - 1 ] );
                --j;
            }
        }
        sideBySide = false;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Refuse( CGridEditReport& report, const CString& reason )
{GUCEF_TRACE;

    report.refused = true;
    report.message = reason;
    m_grid.ReportEdit( CGridView::RangeEditResultEvent, report );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Finish( CGridEditReport& report )
{GUCEF_TRACE;

    m_grid.Touch( EWidgetProperty_Rows );
    m_grid.ReportEdit( CGridView::RangeEditResultEvent, report );
    return !report.refused;
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::Note( CGridEditReport& report, Int32 row, Int32 column, const CString& reason ) const
{GUCEF_TRACE;

    if ( report.problemRow < 0 )
    {
        report.problemRow = row;
        report.problemColumn = column;
        report.message = reason;
    }
}

/*-------------------------------------------------------------------------*/

CString
CGridClipboard::CellText( Int32 row, Int32 column ) const
{GUCEF_TRACE;

    SCellContent content;
    if ( !m_grid.GetCellContent( row, column, content ) )
        return CString::Empty;
    return content.text;
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::PrepareRows( Int32 firstRow, Int32 lastRow ) const
{GUCEF_TRACE;

    if ( m_grid.m_model.IsNULL() || lastRow < firstRow )
        return;
    m_grid.m_model->PrepareRange( CTableViewRange( (UInt32) firstRow, (UInt32) ( lastRow - firstRow + 1 ), 0, m_grid.m_model->GetColumnCount() ) );
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Copy( bool cut )
{GUCEF_TRACE;

    CGridEditReport report( cut ? CGridEditReport::OPERATION_CUT : CGridEditReport::OPERATION_COPY );
    TBlockVector blocks;
    if ( !Resolve( blocks ) )
        return Refuse( report, "Nothing is selected" );
    TBlockVector original = blocks;
    bool sideBySide = true;
    if ( !OrderForCopy( blocks, sideBySide ) )
        return Refuse( report, "The selected ranges do not line up: they need the same rows or the same columns" );

    const CGridColumnLayout& layout = m_grid.GetColumnLayout();
    GUCEF::vector< CString > lines;
    UInt32 cells = 0;
    if ( sideBySide )
    {
        PrepareRows( blocks[ 0 ].firstRow, blocks[ 0 ].lastRow );
        for ( Int32 row = blocks[ 0 ].firstRow; row <= blocks[ 0 ].lastRow; ++row )
        {
            CTabularText::TRow cellsOfRow;
            for ( size_t b=0; b<blocks.size(); ++b )
                for ( Int32 p = blocks[ b ].firstPosition; p <= blocks[ b ].lastPosition; ++p )
                    cellsOfRow.push_back( CellText( row, layout.GetColumn( p ) ) );
            cells += (UInt32) cellsOfRow.size();
            lines.push_back( CTabularText::FormatRow( cellsOfRow ) );
        }
    }
    else
    {
        for ( size_t b=0; b<blocks.size(); ++b )
        {
            PrepareRows( blocks[ b ].firstRow, blocks[ b ].lastRow );
            for ( Int32 row = blocks[ b ].firstRow; row <= blocks[ b ].lastRow; ++row )
            {
                CTabularText::TRow cellsOfRow;
                for ( Int32 p = blocks[ b ].firstPosition; p <= blocks[ b ].lastPosition; ++p )
                    cellsOfRow.push_back( CellText( row, layout.GetColumn( p ) ) );
                cells += (UInt32) cellsOfRow.size();
                lines.push_back( CTabularText::FormatRow( cellsOfRow ) );
            }
        }
    }
    const CString text = CTabularText::JoinLines( lines );
    report.row = blocks[ 0 ].firstRow;
    report.column = layout.GetColumn( blocks[ 0 ].firstPosition );
    if ( !CSecretInput::WriteClipboardText( text ) )
        return Refuse( report, "The clipboard is not available" );
    report.cellsApplied = cells;

    // the cells that were copied are outlined
    m_outline.Clear();
    for ( size_t b=0; b<blocks.size(); ++b )
    {
        if ( 0 == b )
            m_outline.SelectRect( blocks[ b ].firstRow, blocks[ b ].firstPosition, blocks[ b ].lastRow, blocks[ b ].lastPosition );
        else
        {
            m_outline.AddCell( blocks[ b ].firstRow, blocks[ b ].firstPosition );
            m_outline.ExtendTo( blocks[ b ].lastRow, blocks[ b ].lastPosition );
        }
    }
    m_hasOutline = true;

    if ( cut )
    {
        // what was copied is cleared; the report is the one of the clearing
        CGridEditReport cleared( CGridEditReport::OPERATION_CUT );
        cleared.row = report.row;
        cleared.column = report.column;
        if ( !m_grid.m_model.IsNULL() )
        {
            CEditScope scope( m_grid, m_grid.m_model.GetPointerAlways() );
            ClearBlocks( original, cleared );
        }
        else
        {
            cleared.refused = true;
            cleared.message = "The grid has no model to clear the cells in";
            cleared.cellsReadOnly = cells;
        }
        return Finish( cleared );
    }
    return Finish( report );
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::ApplyValue( Int32 row, Int32 column, const CORE::CVariant& value, CGridEditReport& report )
{GUCEF_TRACE;

    const CTableEditResult result = m_grid.ApplyCellEdit( row, column, value );
    if ( result.IsAccepted() )
        ++report.cellsApplied;
    else
    if ( CTableEditResult::STATUS_READ_ONLY == result.status )
    {
        ++report.cellsReadOnly;
        Note( report, row, column, result.message );
    }
    else
    {
        ++report.cellsRefused;
        Note( report, row, column, result.message );
    }
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::ApplyText( Int32 row, Int32 column, const CString& text, CGridEditReport& report )
{GUCEF_TRACE;

    CITableViewModel& model = *m_grid.m_model;
    if ( model.IsCellReadOnly( (UInt32) row, (UInt32) column ) )
    {
        ++report.cellsReadOnly;
        Note( report, row, column, "The cell is read only" );
        return;
    }
    CORE::CVariant current;
    const bool haveCurrent = model.GetCellValue( (UInt32) row, (UInt32) column, current );
    CORE::CVariant value;
    CString error;
    if ( !CTableCellParser::Parse( text, model.GetColumn( (UInt32) column ), haveCurrent ? &current : GUCEF_NULL, value, error ) )
    {
        ++report.cellsRefused;
        Note( report, row, column, error );
        return;
    }
    ApplyValue( row, column, value, report );
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::ApplyConverted( Int32 row, Int32 column, const CORE::CVariant& source, const CTableViewColumn& sourceColumn, CGridEditReport& report )
{GUCEF_TRACE;

    CITableViewModel& model = *m_grid.m_model;
    const CTableViewColumn& target = model.GetColumn( (UInt32) column );
    if ( model.IsCellReadOnly( (UInt32) row, (UInt32) column ) )
    {
        ++report.cellsReadOnly;
        Note( report, row, column, "The cell is read only" );
        return;
    }
    if ( sourceColumn.type == target.type )
    {
        ApplyValue( row, column, source, report );
        return;
    }
    ApplyText( row, column, CTableCellParser::FormatEditText( source, sourceColumn ), report );
}

/*-------------------------------------------------------------------------*/

void
CGridClipboard::ClearBlocks( const TBlockVector& blocks, CGridEditReport& report )
{GUCEF_TRACE;

    const CGridColumnLayout& layout = m_grid.GetColumnLayout();
    CITableViewModel& model = *m_grid.m_model;
    for ( size_t b=0; b<blocks.size(); ++b )
    {
        for ( Int32 p = blocks[ b ].firstPosition; p <= blocks[ b ].lastPosition; ++p )
        {
            const Int32 column = layout.GetColumn( p );
            const CORE::CVariant empty = CTableCellParser::EmptyValue( model.GetColumn( (UInt32) column ) );
            for ( Int32 row = blocks[ b ].firstRow; row <= blocks[ b ].lastRow; ++row )
            {
                if ( model.IsCellReadOnly( (UInt32) row, (UInt32) column ) )
                {
                    ++report.cellsReadOnly;
                    Note( report, row, column, "The cell is read only" );
                    continue;
                }
                ApplyValue( row, column, empty, report );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Clear( void )
{GUCEF_TRACE;

    CGridEditReport report( CGridEditReport::OPERATION_CLEAR );
    TBlockVector blocks;
    if ( m_grid.m_model.IsNULL() )
        return Refuse( report, "The grid has no model to clear the cells in" );
    if ( !Resolve( blocks ) )
        return Refuse( report, "Nothing is selected" );
    report.row = blocks[ 0 ].firstRow;
    report.column = m_grid.GetColumnLayout().GetColumn( blocks[ 0 ].firstPosition );
    {
        CEditScope scope( m_grid, m_grid.m_model.GetPointerAlways() );
        ClearBlocks( blocks, report );
    }
    return Finish( report );
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Paste( void )
{GUCEF_TRACE;

    CGridEditReport report( CGridEditReport::OPERATION_PASTE );
    if ( m_grid.m_model.IsNULL() )
        return Refuse( report, "The grid has no model to paste into" );
    CString text;
    if ( !CSecretInput::ReadClipboardText( text ) || text.IsNULLOrEmpty() )
        return Refuse( report, "The clipboard holds no text" );
    CTabularText::TTable table;
    CTabularText::Parse( text, table );
    if ( table.empty() )
        return Refuse( report, "The clipboard holds no text" );
    TBlockVector targets;
    if ( !Resolve( targets ) )
        return Refuse( report, "Nothing is selected to paste at" );

    const CGridColumnLayout& layout = m_grid.GetColumnLayout();
    const Int32 rows = (Int32) m_grid.GetRowCount();
    const Int32 positions = (Int32) layout.GetPositionCount();
    const Int32 blockRows = (Int32) table.size();
    Int32 blockColumns = 0;
    for ( size_t r=0; r<table.size(); ++r )
        if ( (Int32) table[ r ].size() > blockColumns )
            blockColumns = (Int32) table[ r ].size();

    report.row = targets[ 0 ].firstRow;
    report.column = layout.GetColumn( targets[ 0 ].firstPosition );
    Int32 selectRows = 0, selectPositions = 0;
    {
        CEditScope scope( m_grid, m_grid.m_model.GetPointerAlways() );
        if ( 1 == blockRows && 1 == blockColumns )
        {
            // one cell of text goes to every selected cell
            for ( size_t b=0; b<targets.size(); ++b )
                for ( Int32 row = targets[ b ].firstRow; row <= targets[ b ].lastRow; ++row )
                    for ( Int32 p = targets[ b ].firstPosition; p <= targets[ b ].lastPosition; ++p )
                        ApplyText( row, layout.GetColumn( p ), table[ 0 ].empty() ? CString::Empty : table[ 0 ][ 0 ], report );
        }
        else
        {
            if ( targets.size() > 1 )
                return Refuse( report, "A block of cells cannot be pasted into several ranges: select one cell" );
            const SBlock& target = targets[ 0 ];
            const Int32 targetRows = target.lastRow - target.firstRow + 1;
            const Int32 targetColumns = target.lastPosition - target.firstPosition + 1;
            const bool repeat = 0 == targetRows % blockRows && 0 == targetColumns % blockColumns;
            const Int32 pasteRows = repeat ? targetRows : blockRows;
            const Int32 pasteColumns = repeat ? targetColumns : blockColumns;
            selectRows = pasteRows;
            selectPositions = pasteColumns;
            PrepareRows( target.firstRow, target.firstRow + pasteRows - 1 );
            for ( Int32 dr=0; dr<pasteRows; ++dr )
            {
                const Int32 row = target.firstRow + dr;
                const CTabularText::TRow& source = table[ (size_t) ( dr % blockRows ) ];
                for ( Int32 dc=0; dc<pasteColumns; ++dc )
                {
                    const Int32 position = target.firstPosition + dc;
                    const size_t sourceColumn = (size_t) ( dc % blockColumns );
                    if ( row >= rows || position >= positions || sourceColumn >= source.size() )
                    {
                        ++report.cellsSkipped;
                        continue;
                    }
                    ApplyText( row, layout.GetColumn( position ), source[ sourceColumn ], report );
                }
            }
        }
    }

    // the pasted cells are the selection
    if ( selectRows > 0 && 1 == targets.size() )
    {
        const SBlock& target = targets[ 0 ];
        Int32 lastRow = target.firstRow + selectRows - 1;
        Int32 lastPosition = target.firstPosition + selectPositions - 1;
        if ( lastRow >= rows ) lastRow = rows - 1;
        if ( lastPosition >= positions ) lastPosition = positions - 1;
        m_grid.SelectRange( target.firstRow, layout.GetColumn( target.firstPosition ), lastRow, layout.GetColumn( lastPosition ) );
    }
    if ( 0 == report.cellsApplied && ( report.cellsRefused > 0 || report.cellsReadOnly > 0 ) )
        report.refused = true;
    return Finish( report );
}

/*-------------------------------------------------------------------------*/

bool
CGridClipboard::Fill( bool down )
{GUCEF_TRACE;

    CGridEditReport report( down ? CGridEditReport::OPERATION_FILL_DOWN : CGridEditReport::OPERATION_FILL_RIGHT );
    if ( m_grid.m_model.IsNULL() )
        return Refuse( report, "The grid has no model to fill" );
    TBlockVector blocks;
    if ( !Resolve( blocks ) )
        return Refuse( report, "Nothing is selected" );
    const CGridColumnLayout& layout = m_grid.GetColumnLayout();
    CITableViewModel& model = *m_grid.m_model;
    report.row = blocks[ 0 ].firstRow;
    report.column = layout.GetColumn( blocks[ 0 ].firstPosition );
    {
        CEditScope scope( m_grid, m_grid.m_model.GetPointerAlways() );
        for ( size_t b=0; b<blocks.size(); ++b )
        {
            const SBlock& block = blocks[ b ];
            if ( down )
            {
                // the first row goes over the others; one row takes the row above it
                const bool single = block.firstRow == block.lastRow;
                const Int32 sourceRow = single ? block.firstRow - 1 : block.firstRow;
                const Int32 firstTarget = single ? block.firstRow : block.firstRow + 1;
                for ( Int32 p = block.firstPosition; p <= block.lastPosition; ++p )
                {
                    const Int32 column = layout.GetColumn( p );
                    CORE::CVariant source;
                    if ( sourceRow < 0 || !model.GetCellValue( (UInt32) sourceRow, (UInt32) column, source ) )
                    {
                        report.cellsSkipped += (UInt32) ( block.lastRow - firstTarget + 1 );
                        continue;
                    }
                    for ( Int32 row = firstTarget; row <= block.lastRow; ++row )
                        ApplyConverted( row, column, source, model.GetColumn( (UInt32) column ), report );
                }
            }
            else
            {
                const bool single = block.firstPosition == block.lastPosition;
                const Int32 sourcePosition = single ? block.firstPosition - 1 : block.firstPosition;
                const Int32 firstTarget = single ? block.firstPosition : block.firstPosition + 1;
                for ( Int32 row = block.firstRow; row <= block.lastRow; ++row )
                {
                    CORE::CVariant source;
                    const Int32 sourceColumn = sourcePosition >= 0 ? layout.GetColumn( sourcePosition ) : -1;
                    if ( sourceColumn < 0 || !model.GetCellValue( (UInt32) row, (UInt32) sourceColumn, source ) )
                    {
                        report.cellsSkipped += (UInt32) ( block.lastPosition - firstTarget + 1 );
                        continue;
                    }
                    for ( Int32 p = firstTarget; p <= block.lastPosition; ++p )
                        ApplyConverted( row, layout.GetColumn( p ), source, model.GetColumn( (UInt32) sourceColumn ), report );
                }
            }
        }
    }
    if ( 0 == report.cellsApplied && ( report.cellsRefused > 0 || report.cellsReadOnly > 0 ) )
        report.refused = true;
    return Finish( report );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
