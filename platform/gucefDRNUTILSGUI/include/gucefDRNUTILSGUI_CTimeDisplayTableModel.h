/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYTABLEMODEL_H
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYTABLEMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDATATABLEVIEWMODEL_H
#include "gucefGUI_CDataTableViewModel.h"
#define GUCEF_GUI_CDATATABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CDATATABLEVIEWMODEL_H ? */

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYFORMATTER_H
#include "gucefDRNUTILSGUI_CTimeDisplayFormatter.h"
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYFORMATTER_H
#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYFORMATTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a form says about a column it shows of a table: the name in the table, the title and the width in pixels (0: the grid chooses) */
struct SGridColumnSpec
{
    const char* name;
    const char* title;
    Int32 width;
};

/*-------------------------------------------------------------------------*/

/**
 *  The model a grid of the forms shows a table with: a CDataTableViewModel that follows the changes of the table, with the
 *  columns the form chose (the table's presentation hints win over the choice of the form: hint "hidden" takes a column
 *  away, "title", "width", "alignment", "decimals", "thousandsSeparator", "prefix", "suffix", "booleanDisplay" and
 *  "nullText" replace those of the column) and the time display of 11.4: a timestamp cell is shown in the display zone of
 *  the settings with its zone suffix. Only the text of the cell is translated: GetCellValue(), the sort order and the filters
 *  stay on the UTC value of the table, and changing the display zone changes no data.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CTimeDisplayTableModel : public GUI::CDataTableViewModel
{
    public:

    /** 'specs' are the columns shown, in order (columns that the table does not have are left out); no specs: every column of the table */
    CTimeDisplayTableModel( const CORE::CIDataTablePtr& table         ,
                            const SGridColumnSpec* specs              ,
                            UInt32 specCount                          ,
                            const CTimeDisplaySettings& timeDisplay   );

    virtual ~CTimeDisplayTableModel() GUCEF_VIRTUAL_OVERRIDE;

    /** Shows the timestamps in another zone or format: the model is reset, the grid redraws */
    void SetTimeDisplay( const CTimeDisplaySettings& timeDisplay );

    const CTimeDisplayFormatter& GetFormatter( void ) const;

    /** The index of the shown column of the name, GUCEF_UINT32MAX when it is not shown */
    UInt32 FindShownColumn( const CORE::CString& columnName ) const;

    virtual const GUI::CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetCellContent( UInt32 row, UInt32 column, GUI::SCellContent& content ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The UTC text of a timestamp cell ("2026-11-15T13:05:00Z"), empty for any other cell; what a tooltip of the cell shows */
    CORE::CString GetCellUtcText( UInt32 row, UInt32 column ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTimeDisplayTableModel( const CTimeDisplayTableModel& src );                /**< not implemented */
    CTimeDisplayTableModel& operator=( const CTimeDisplayTableModel& src );     /**< not implemented */

    bool GetTimestampMs( UInt32 row, UInt32 column, Int64& utcMs ) const;

    GUCEF::vector< GUI::CTableViewColumn > m_presented;
    CTimeDisplayFormatter m_formatter;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYTABLEMODEL_H ? */
