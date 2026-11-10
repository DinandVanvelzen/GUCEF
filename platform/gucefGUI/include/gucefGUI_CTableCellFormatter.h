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

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#define GUCEF_GUI_CTABLECELLFORMATTER_H

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

/** What a cell shows: text, the distinct empty cell of a null, or a check box that is ticked or not */
struct SCellContent
{
    enum EKind
    {
        KIND_TEXT = 0   ,
        KIND_NULL       ,
        KIND_CHECK_ON   ,
        KIND_CHECK_OFF
    };

    EKind kind;
    CString text;       /**< KIND_TEXT: the text; KIND_NULL: the null text of the column; a check: "true" or "false" */
    bool isNumber;      /**< the value is a number: it is aligned and measured as one */

    SCellContent( void ) : kind( KIND_TEXT ), text(), isNumber( false ) {}
};

/**
 *  Turns the typed value of a cell into what the grid shows, by the definition of the column: integers with or
 *  without the digits grouped, floats and decimals rounded half up to the decimals of the column, dates and
 *  timestamps by a pattern (UTC), booleans as a check mark or as text, a null as the distinct empty cell, a prefix
 *  and a suffix around numbers. Stateless, and every member is reentrant.
 */
class GUCEF_GUI_PUBLIC_CPP CTableCellFormatter
{
    public:

    /** The content of a cell */
    static void Format( const CORE::CVariant& value, const CTableViewColumn& column, SCellContent& content );

    /** The text of a cell: the text of the content; "true" or "false" for a boolean shown as a check mark, the null text for a null */
    static CString FormatText( const CORE::CVariant& value, const CTableViewColumn& column );

    /** A day count since 1970-01-01 by a pattern; empty pattern: yyyy-MM-dd */
    static CString FormatDate( Int32 days, const CString& pattern );

    /** Ticks since the unix epoch (UTC) of a unit by a pattern; empty pattern: yyyy-MM-dd HH:mm:ss */
    static CString FormatTimestamp( Int64 ticks, CORE::TDataTableTimeUnit unit, const CString& pattern );

    /** Groups the digits before the decimal point of a number written as text in threes, with a comma: 1234567.5 gives 1,234,567.5 */
    static CString GroupDigits( const CString& number );

    private:

    CTableCellFormatter( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */
