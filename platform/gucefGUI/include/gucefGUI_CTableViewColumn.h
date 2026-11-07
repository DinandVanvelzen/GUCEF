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

#ifndef GUCEF_GUI_CTABLEVIEWCOLUMN_H
#define GUCEF_GUI_CTABLEVIEWCOLUMN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
 *  What a table view model says about one of its columns: the title, the logical type of its values (the types
 *  of the data tables, so a table schema maps one to one), the width, whether the user may change it and how the
 *  values are shown. The way the values are shown is part of the column, not of the cell: every optional member
 *  has a default that shows a value the way its type says.
 */
class GUCEF_GUI_PUBLIC_CPP CTableViewColumn
{
    public:

    /** How a boolean is shown: a check mark (a box with a tick, nothing for false) or the text true and false */
    enum EBooleanDisplay
    {
        BOOLEAN_CHECK = 0   ,
        BOOLEAN_TEXT
    };

    CString name;                       /**< the key of the column in the model (the column of a table); the title when empty */
    CString title;                      /**< shown in the header */
    CORE::TDataTableLogicalType type;   /**< default DATATABLE_TYPE_UTF8 */
    Int32 width;                        /**< pixels, 0 for the width the grid chooses */
    bool readOnly;                      /**< default true */
    CString alignment;                  /**< "Left", "Center" or "Right"; empty for the alignment of the type: numbers, dates and times right, booleans in the middle, the rest left */
    Int32 decimals;                     /**< digits after the decimal point of a float or decimal, rounded half up; -1 for the digits the value has */
    bool thousandsSeparator;            /**< groups the digits of a number in threes with a comma */
    CString prefix;                     /**< put before a number, for example a currency sign */
    CString suffix;                     /**< put after a number, for example a percent sign */
    CString dateFormat;                 /**< for dates and timestamps: yyyy, MM, dd, HH, mm, ss and fff (milliseconds) stand for the parts; empty for yyyy-MM-dd of a date and yyyy-MM-dd HH:mm:ss of a timestamp (UTC) */
    EBooleanDisplay booleanDisplay;
    CString nullText;                   /**< shown in the distinct empty cell of a null; default empty */
    GUCEF::vector< CString > choices;   /**< the values a user may pick from when the cell is edited (as text, read by the type of the column); empty for a free value */

    CTableViewColumn( void );

    CTableViewColumn( const CString& columnTitle, CORE::TDataTableLogicalType columnType, Int32 columnWidth = 0, bool isReadOnly = true );

    /** The title, or the name when there is no title */
    const CString& GetDisplayTitle( void ) const;

    /** True for the integer, float and decimal types */
    bool IsNumeric( void ) const;

    /** True for dates and timestamps */
    bool IsTemporal( void ) const;

    /** The alignment that is shown: 'alignment', or the one the type says */
    CString GetEffectiveAlignment( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABLEVIEWCOLUMN_H ? */
