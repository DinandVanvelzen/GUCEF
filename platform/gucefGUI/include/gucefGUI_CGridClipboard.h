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
#ifndef GUCEF_GUI_CGRIDCLIPBOARD_H
#define GUCEF_GUI_CGRIDCLIPBOARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

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
 *  Copy, cut, paste, fill and clear of the selected cells of a grid (CGridView, property CellClipboard). The cells go to
 *  the clipboard as tab separated text (CTabularText) with the text the grid shows, the clipboard is the one of
 *  CSecretInput (SetClipboardReader() and SetClipboardWriter()), and what comes back is read by the type of the column
 *  of every cell (CTableCellParser) and given to the model cell by cell, through the validation of the model
 *  (CGridView::ApplyCellEdit). The grid owns one when it first needs it.
 *
 *  Several ranges are copied the way a spreadsheet does it: when every range has the same rows they are put side by side
 *  (in the order of their columns), when every range has the same columns they are put under each other (in the order of
 *  their rows); anything else is refused and the refusal is reported. Hidden columns are not copied.
 *
 *  A paste starts at the top left cell of the selection. One cell of text fills every selected cell; a block of text
 *  goes to one range, and is repeated over it when the range is a whole multiple of the block. Cells outside the grid
 *  are left out; cells the model says are read only, cells whose text is no value of the column and cells the model
 *  refuses are counted and the first reason is kept; the others are applied. Fill down copies the first row of every range
 *  over the others (the row above the cell when one row is selected), fill right the first column. Every operation
 *  reports with RangeEditResultEvent and keeps the model's edit group open around the cells it changes, so that a model
 *  with a history undoes the operation in one step.
 *
 *  After a copy or a cut the copied cells are outlined until the next change of the rows, Escape, or a new copy.
 */
class GUCEF_GUI_PUBLIC_CPP CGridClipboard
{
    public:

    explicit CGridClipboard( CGridView& grid );

    ~CGridClipboard();

    bool Copy( bool cut );

    bool Paste( void );

    bool Fill( bool down );

    bool Clear( void );

    /** The cells copied last, NULL when there is no outline */
    const CGridSelection* GetOutline( void ) const;

    void ClearOutline( void );

    /** The rows or columns of the model changed: the outline no longer says which cells were copied */
    void OnModelChanged( void );

    private:

    /** A rectangle of cells: rows and the places of the shown columns, both ends included */
    struct SBlock
    {
        Int32 firstRow;
        Int32 lastRow;
        Int32 firstPosition;
        Int32 lastPosition;
    };
    typedef GUCEF::vector< SBlock > TBlockVector;

    bool Resolve( TBlockVector& blocks ) const;
    bool OrderForCopy( TBlockVector& blocks, bool& sideBySide ) const;
    bool Refuse( CGridEditReport& report, const CString& reason );
    bool Finish( CGridEditReport& report );
    void Note( CGridEditReport& report, Int32 row, Int32 column, const CString& reason ) const;
    void ApplyValue( Int32 row, Int32 column, const CORE::CVariant& value, CGridEditReport& report );
    void ApplyText( Int32 row, Int32 column, const CString& text, CGridEditReport& report );
    void ApplyConverted( Int32 row, Int32 column, const CORE::CVariant& source, const CTableViewColumn& sourceColumn, CGridEditReport& report );
    void ClearBlocks( const TBlockVector& blocks, CGridEditReport& report );
    CString CellText( Int32 row, Int32 column ) const;
    void PrepareRows( Int32 firstRow, Int32 lastRow ) const;

    CGridClipboard( const CGridClipboard& src );
    CGridClipboard& operator=( const CGridClipboard& src );

    CGridView& m_grid;
    CGridSelection m_outline;
    bool m_hasOutline;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCLIPBOARD_H ? */
