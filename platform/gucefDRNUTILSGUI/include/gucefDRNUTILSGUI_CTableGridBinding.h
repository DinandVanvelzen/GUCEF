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

#ifndef GUCEF_DRNUTILSGUI_CTABLEGRIDBINDING_H
#define GUCEF_DRNUTILSGUI_CTABLEGRIDBINDING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYTABLEMODEL_H
#include "gucefDRNUTILSGUI_CTimeDisplayTableModel.h"
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYTABLEMODEL_H
#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYTABLEMODEL_H ? */

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

/**
 *  A grid of a form bound to a table: the model (CTimeDisplayTableModel) follows the table, the grid shows the model, and
 *  the binding tells the form when the rows changed (ModelChangedEvent, after every RowsChangedEvent of the table) and when
 *  the user selected another row (SelectionChangedEvent). The form reads the selection as the text of a cell of the selected
 *  row, normally of the key column, so it does not depend on the order the user sorted the grid in.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CTableGridBinding : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ModelChangedEvent;            /**< no data: the rows or the columns of the table changed; the grid shows the new state */
    static const CORE::CEvent SelectionChangedEvent;        /**< no data: another row was selected (or the selected row was lost) */

    static void RegisterEvents( void );

    CTableGridBinding( void );

    virtual ~CTableGridBinding() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Shows the table in the grid. A NULL table leaves the grid empty. The grid is not owned and has to outlive the binding,
     *  or the binding has to be unbound first. The column specs are copied. False when the table cannot be shown yet (it
     *  has no columns that can be shown, as a DRN replica before its schema arrived): the binding then waits for the
     *  table's rows or schema to change and shows it from then on (ModelChangedEvent). A table whose schema changes
     *  (CDRNPublishedTable::SchemaChangedEvent) is shown with a new model, keeping the sort and the filter
     */
    bool Bind( GUI::CGridView* grid                  ,
               const CORE::CIDataTablePtr& table     ,
               const CORE::CString& keyColumn        ,
               const SGridColumnSpec* specs          ,
               UInt32 specCount                      ,
               const CTimeDisplaySettings& timeDisplay );

    void Unbind( void );

    bool IsBound( void ) const;

    const CORE::CString& GetKeyColumn( void ) const;

    void SetTimeDisplay( const CTimeDisplaySettings& timeDisplay );

    /** Shows only the rows of the table that match (AND'ed); an empty vector shows all. The sort of the grid stays */
    bool SetFilter( const CORE::CDataTableQuery::TPredicateVector& predicates );

    /** Orders the rows by the shown column when the model can; false when it cannot */
    bool SortBy( const CORE::CString& columnName, bool ascending );

    UInt32 GetRowCount( void ) const;

    /** The text of a shown cell of a row of the grid (the way the grid shows it) */
    bool GetCellText( UInt32 row, const CORE::CString& columnName, CORE::CString& text ) const;

    /** The value of a cell of a row of the grid, as the table holds it (UTC for timestamps, whatever the display zone is) */
    bool GetCellValue( UInt32 row, const CORE::CString& columnName, CORE::CVariant& value ) const;

    /** The selected row of the grid, -1 for none */
    Int32 GetSelectedRow( void ) const;

    bool GetSelectedCellValue( const CORE::CString& columnName, CORE::CVariant& value ) const;

    /** The text of the cell of the key column of the selected row, empty without selection */
    CORE::CString GetSelectedKeyText( void ) const;

    /** Selects the row whose key column has the text, as the user clicking it does; false when no row has it */
    bool SelectKeyText( const CORE::CString& keyText );

    void ClearSelection( void );

    const CTimeDisplayTableModel* GetModel( void ) const;

    GUI::CGridView* GetGrid( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTableGridBinding > TEventCallback;

    CTableGridBinding( const CTableGridBinding& src );                /**< not implemented */
    CTableGridBinding& operator=( const CTableGridBinding& src );     /**< not implemented */

    void OnModelReset( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void OnGridSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void OnTableShapeChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /** Makes the model of the bound table with the kept columns, time display, sort and filter; false without columns */
    bool BuildModel( void );

    GUI::CGridView* m_grid;
    GUI::TTableViewModelPtr m_model;
    CTimeDisplayTableModel* m_timeModel;        /**< the model as its own class, owned by m_model */
    CORE::CString m_keyColumn;
    CORE::CIDataTablePtr m_table;
    GUCEF::vector< SGridColumnSpec > m_specs;
    CTimeDisplaySettings m_timeDisplay;
    bool m_hasSort;
    CORE::CString m_sortColumn;
    bool m_sortAscending;
    CORE::CDataTableQuery::TPredicateVector m_filter;
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

#endif /* GUCEF_DRNUTILSGUI_CTABLEGRIDBINDING_H ? */
