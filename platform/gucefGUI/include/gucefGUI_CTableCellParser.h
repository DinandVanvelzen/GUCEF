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
#ifndef GUCEF_GUI_CTABLECELLPARSER_H
#define GUCEF_GUI_CTABLECELLPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_GUI_CTABLEVIEWCOLUMN_H
#include "gucefGUI_CTableViewColumn.h"
#define GUCEF_GUI_CTABLEVIEWCOLUMN_H
#endif /* GUCEF_GUI_CTABLEVIEWCOLUMN_H ? */

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
 *  The other way round of CTableCellFormatter: the text a user typed, or pasted, in a cell becomes a typed value by
 *  the definition of the column. Numbers may carry the prefix, the suffix and the grouping commas of the column,
 *  dates and timestamps are read by the pattern of the column (or as ISO 8601), booleans from true, false, yes, no,
 *  1 and 0. The text of a cell that is empty after trimming is a null for every type but text. A text that does not fit
 *  the type gives false and a reason a person can read. Stateless, every member is reentrant.
 */
class GUCEF_GUI_PUBLIC_CPP CTableCellParser
{
    public:

    /**
     *  The value of the text. 'current' (may be NULL) is the value now in the cell: a timestamp is read in the time unit of
     *  it, milliseconds without one. 'error' is set when false is returned
     */
    static bool Parse( const CString& text, const CTableViewColumn& column, const CORE::CVariant* current, CORE::CVariant& value, CString& error );

    /** The value of a cell that was cleared: the empty text for a text column, a null for every other */
    static CORE::CVariant EmptyValue( const CTableViewColumn& column );

    /**
     *  The text an editor starts with: the value in full precision, with no prefix, suffix or grouping of the column, which
     *  Parse() reads back as the same value. A null is the empty text
     */
    static CString FormatEditText( const CORE::CVariant& value, const CTableViewColumn& column );

    /** Whether the values of the column can be typed: not the binary and the unsupported types */
    static bool IsEditableType( const CTableViewColumn& column );

    /** What a person can type in a cell of the column: the pattern for a date, "number", "text" ... as a hint for an empty editor */
    static CString GetTypeHint( const CTableViewColumn& column );

    private:

    CTableCellParser( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABLECELLPARSER_H ? */
