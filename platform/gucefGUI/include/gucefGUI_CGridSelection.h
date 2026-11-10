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

#ifndef GUCEF_GUI_CGRIDSELECTION_H
#define GUCEF_GUI_CGRIDSELECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The cells a grid has selected in its cell mode: a list of rectangles (a click makes one, Ctrl + click adds one,
 *  Shift + click or a drag changes the last one), the anchor the last rectangle was started at and the corner of it
 *  that moves when it is extended. A rectangle spans rows [firstRow, lastRow] and the positions [firstColumn,
 *  lastColumn] of the shown columns (the place from the left after the order and the hidden columns); a rectangle
 *  of whole rows covers every column, a rectangle of whole columns every row, both together everything.
 *
 *  This is the bookkeeping of the selection and nothing else: the grid decides what a click or a key selects.
 */
class GUCEF_GUI_PUBLIC_CPP CGridSelection
{
    public:

    struct SRange
    {
        Int32 firstRow;
        Int32 lastRow;
        Int32 firstColumn;
        Int32 lastColumn;
        bool wholeRows;             /**< every column of the rows */
        bool wholeColumns;          /**< every row of the columns */
    };
    typedef GUCEF::vector< SRange > TRangeVector;

    CGridSelection( void );

    bool IsEmpty( void ) const;

    void Clear( void );

    UInt32 GetRangeCount( void ) const;

    const SRange& GetRange( UInt32 index ) const;

    const TRangeVector& GetRanges( void ) const;

    /** Whether a cell (a row and the position of a shown column) is in a rectangle */
    bool Contains( Int32 row, Int32 position ) const;

    /** Whether every cell of the row is selected: by a rectangle of whole rows or of whole columns together with others */
    bool ContainsWholeRow( Int32 row ) const;

    bool ContainsWholeColumn( Int32 position ) const;

    /** Whether a row has a selected cell */
    bool TouchesRow( Int32 row ) const;

    /** The anchor of the last rectangle, -1 when there is none */
    Int32 GetAnchorRow( void ) const;

    Int32 GetAnchorPosition( void ) const;

    /** The corner that moves when the last rectangle is extended */
    Int32 GetExtentRow( void ) const;

    Int32 GetExtentPosition( void ) const;

    /** One cell is all there is */
    void SelectCell( Int32 row, Int32 position );

    /** One rectangle with the anchor at the first corner */
    void SelectRect( Int32 anchorRow, Int32 anchorPosition, Int32 row, Int32 position );

    /** Adds a rectangle of one cell; the anchor moves to it */
    void AddCell( Int32 row, Int32 position );

    /** Changes the last rectangle to run from its anchor to the cell, a rectangle of whole rows or columns stays one; one cell is made when there is none */
    void ExtendTo( Int32 row, Int32 position );

    /** Rows [first, last] whole: replaces everything, or adds one when 'add' is set. The anchor is the first row */
    void SelectRows( Int32 anchorRow, Int32 row, bool add );

    /** Columns whole, by position; the same way */
    void SelectColumns( Int32 anchorPosition, Int32 position, bool add );

    void SelectAll( void );

    /** Rows were inserted before 'firstRow' / removed from 'firstRow' on: rectangles move or shrink, those that lose all their rows go */
    void OnRowsInserted( Int32 firstRow, Int32 count );

    void OnRowsRemoved( Int32 firstRow, Int32 count );

    /** Keeps what fits in 'rowCount' rows and 'positionCount' positions */
    void Clamp( Int32 rowCount, Int32 positionCount );

    /** One line per rectangle, "firstRow,firstColumn,lastRow,lastColumn"; whole rows have the columns 0,-1 and whole columns the rows 0,-1 */
    GUCEF::vector< CString > ToText( void ) const;

    bool operator==( const CGridSelection& other ) const;

    bool operator!=( const CGridSelection& other ) const;

    private:

    static SRange MakeRange( Int32 rowA, Int32 positionA, Int32 rowB, Int32 positionB );

    TRangeVector m_ranges;
    Int32 m_anchorRow;
    Int32 m_anchorPosition;
    Int32 m_extentRow;
    Int32 m_extentPosition;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDSELECTION_H ? */
