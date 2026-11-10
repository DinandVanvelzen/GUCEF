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

#ifndef GUCEF_GUI_CGRIDCOLUMNLAYOUT_H
#define GUCEF_GUI_CGRIDCOLUMNLAYOUT_H

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
 *  Where the columns of a grid stand: which columns are shown, in which order, how wide, which of them are frozen
 *  at the left, and what the horizontal scroll position shows of the rest. The columns are the columns of the data
 *  (model columns); a position is the place of a shown column from the left, after the order and the hidden
 *  columns were applied. With nothing hidden, no order, no frozen columns, no row header and no scroll the layout
 *  is the plain row of columns, the position of a column is its index and its left is the sum of the widths before it.
 *
 *  The coordinates of a position are of two kinds: the content coordinate (the left of the column in the row of all
 *  shown columns, before the scroll) and the screen coordinate (the left in the grid after the row header, the scroll
 *  and the frozen columns, which do not scroll).
 */
class GUCEF_GUI_PUBLIC_CPP CGridColumnLayout
{
    public:

    typedef GUCEF::vector< Int32 > TInt32Vector;

    CGridColumnLayout( void );

    /**
     *  Makes the layout. 'baseWidths' holds the width in pixels of every column of the data; 'order' the data
     *  columns in the order they are shown (columns that are missing or out of range follow in their own order, a
     *  repeated one counts once); 'hidden' the columns that are not shown; 'frozenColumns' how many of the first
     *  shown columns do not scroll (at most all but one); 'rowHeaderWidth' the room at the left for the row header;
     *  'viewWidth' the width of the grid; 'scrollX' the pixels the columns that scroll are moved to the left
     */
    void Compute( const TInt32Vector& baseWidths ,
                  const TInt32Vector& order      ,
                  const TInt32Vector& hidden     ,
                  Int32 frozenColumns            ,
                  Int32 rowHeaderWidth           ,
                  Int32 viewWidth                ,
                  Int32 scrollX                  );

    /** The shown columns */
    UInt32 GetPositionCount( void ) const;

    /** The data column at a position, -1 for a position that is not there */
    Int32 GetColumn( Int32 position ) const;

    /** The position of a data column, -1 when it is hidden or not there */
    Int32 FindPosition( Int32 column ) const;

    Int32 GetWidth( Int32 position ) const;

    /** The content coordinate of the left of a position */
    Int32 GetContentLeft( Int32 position ) const;

    /** The left of a position in the grid, after the row header, the scroll and the frozen columns */
    Int32 GetScreenLeft( Int32 position ) const;

    /** Whether the position is in the frozen columns */
    bool IsFrozen( Int32 position ) const;

    Int32 GetFrozenCount( void ) const;

    /** The width of the frozen columns */
    Int32 GetFrozenWidth( void ) const;

    Int32 GetRowHeaderWidth( void ) const;

    /** The width of all shown columns */
    Int32 GetContentWidth( void ) const;

    /** The width the columns that scroll are shown in: the grid without the row header and the frozen columns */
    Int32 GetScrollViewWidth( void ) const;

    /** The scroll position, held between 0 and GetMaxScrollX() */
    Int32 GetScrollX( void ) const;

    /** The width of the columns that scroll, less what shows: 0 when they fit */
    Int32 GetMaxScrollX( void ) const;

    /** The position of the column under an x of the grid, -1 for the row header, the space after the last column and the like */
    Int32 PositionAt( Int32 x ) const;

    /** The positions of the columns that scroll that show, at least in part: [first, last], first > last for none */
    void GetVisibleScrolledPositions( Int32& first, Int32& last ) const;

    /** The scroll position that shows a position, changing the current one the least; the position of a frozen column needs none */
    Int32 GetScrollToShow( Int32 position ) const;

    private:

    TInt32Vector m_columns;         /**< the data column of every position */
    TInt32Vector m_lefts;           /**< the content left of every position */
    TInt32Vector m_widths;
    TInt32Vector m_positionOf;      /**< the position of every data column, -1 hidden */
    Int32 m_frozenCount;
    Int32 m_frozenWidth;
    Int32 m_rowHeaderWidth;
    Int32 m_contentWidth;
    Int32 m_viewWidth;
    Int32 m_scrollX;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCOLUMNLAYOUT_H ? */
