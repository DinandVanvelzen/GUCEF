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

#ifndef GUCEF_GUI_CGRIDCOLUMNCHOOSERFORM_H
#define GUCEF_GUI_CGRIDCOLUMNCHOOSERFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#include "gucefGUI_CITableViewModel.h"
#define GUCEF_GUI_CITABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */

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

class CGridView;
class CGridColumnChooserModel;

/**
 *  The column chooser of a grid: a modal window that lists every column the grid has, also the hidden ones and the
 *  columns the model offers hidden by default, in the order they stand. A tick shows or hides a column (a required
 *  column, and the last column shown, keep their tick), Move up and Move down change the order (hidden columns too),
 *  Reset to default goes back to what the application set up; every change is applied to the grid at once (and kept
 *  by its preference store). Close and Escape close it. The grid opens it from the button of its header strip
 *  (CGridView::OpenColumnChooser()); it is built in code: a Window "GridColumnChooserForm" with a GridView "Columns"
 *  (Shown, Column, Note) and the buttons "MoveUp", "MoveDown", "Reset", "Close".
 */
class GUCEF_GUI_PUBLIC_CPP CGridColumnChooserForm : public CFormEx
{
    public:

    static const CORE::CEvent ClosedEvent;

    static void RegisterEvents( void );

    CGridColumnChooserForm( CIGUIContext* context );

    virtual ~CGridColumnChooserForm();

    /** Makes the window; Open() does this when it was not done */
    bool Build( void );

    /** Lists the columns of the grid and shows the window, modal */
    bool Open( CGridView& grid );

    void Close( void );

    bool IsOpen( void ) const;

    /** The grid whose columns are listed, NULL before Open() */
    CGridView* GetGrid( void ) const;

    /** The list of the columns */
    CGridView* GetColumnGrid( void );

    /** The rows of the list: the columns of the data of the grid in order */
    UInt32 GetRowCount( void ) const;

    /** The column of the data of the grid a row of the list stands for, -1 for none */
    Int32 GetGridColumn( UInt32 row ) const;

    /** Shows or hides the column of a row as its tick does; false when it cannot be hidden */
    bool SetColumnShown( UInt32 row, bool shown );

    /** Moves the column of a row one place up or down in the order; the cursor of the list goes with it */
    bool MoveRow( UInt32 row, bool up );

    /** As the Reset to default button */
    void ResetToDefault( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CGridColumnChooserForm > TEventCallback;

    void OnMoveUpActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMoveDownActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridColumnsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );
    void MoveCursorRow( bool up );

    CGridColumnChooserForm( const CGridColumnChooserForm& src );
    CGridColumnChooserForm& operator=( const CGridColumnChooserForm& src );

    private:

    CGridView* m_grid;
    CGridColumnChooserModel* m_model;
    TTableViewModelPtr m_modelPtr;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCOLUMNCHOOSERFORM_H ? */
