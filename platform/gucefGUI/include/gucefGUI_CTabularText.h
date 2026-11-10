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
#ifndef GUCEF_GUI_CTABULARTEXT_H
#define GUCEF_GUI_CTABULARTEXT_H

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
 *  Rows of cells as tab separated text, the text a spreadsheet puts on the clipboard: cells are separated by a tab,
 *  rows by a line feed. A cell that holds a tab, a line feed, a carriage return or a double quote is written in double
 *  quotes with its own double quotes doubled. Reading takes line feed, carriage return line feed and carriage return
 *  as the end of a row, understands quoted cells (which may hold tabs and line ends) and ignores the line end at the
 *  very end of the text. A last row that is one empty cell would vanish with that line end, so such a text gets one more
 *  line end (EndsInEmptyCell()). Stateless.
 */
class GUCEF_GUI_PUBLIC_CPP CTabularText
{
    public:

    typedef GUCEF::vector< CString > TRow;
    typedef GUCEF::vector< TRow > TTable;

    /** Whether the last row is one cell without text: the text of the rows then ends in a line end of its own */
    static bool EndsInEmptyCell( const TTable& rows );

    /** The cell as it is written in the text: quoted when it has to be */
    static CString QuoteCell( const CString& text );

    /** The rows as text: no line end after the last row */
    static CString Format( const TTable& rows );

    /** The text of one row: the cells quoted where it is needed, separated by a tab */
    static CString FormatRow( const TRow& row );

    /** The lines (see FormatRow()) one under the other, in one allocation: a text of many rows is not made by appending to it. The one more line end of EndsInEmptyCell() is added when the last line is empty */
    static CString JoinLines( const GUCEF::vector< CString >& lines );

    /** The rows of the text; no rows for empty text. False when a quote is not closed (the rest of the text is then the last cell) */
    static bool Parse( const CString& text, TTable& rows );

    private:

    CTabularText( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABULARTEXT_H ? */
