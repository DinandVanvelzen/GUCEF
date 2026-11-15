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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H
#include "gucefGUI_CGridColumnChooserModel.h"
#define GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H
#endif /* GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H ? */

#include "gucefGUI_CGridColumnChooserForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CGridColumnChooserForm::ClosedEvent = "GUCEF::GUI::CGridColumnChooserForm::ClosedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGridColumnChooserForm::CGridColumnChooserForm( CIGUIContext* context )
    : CFormEx( context )
    , m_grid( GUCEF_NULL )
    , m_model( GUCEF_NULL )
    , m_modelPtr()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_model = GUCEF_NEW CGridColumnChooserModel();
    m_modelPtr = TTableViewModelPtr( m_model );
}

/*-------------------------------------------------------------------------*/

CGridColumnChooserForm::~CGridColumnChooserForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    CGridView* list = GetColumnGrid();
    if ( GUCEF_NULL != list )
        list->SetModel( TTableViewModelPtr() );
    m_model->SetGrid( GUCEF_NULL );
    m_grid = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "GridColumnChooserForm", "0.5,-240 0.5,-250", "0,480 0,500" );
    CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, "Columns" );
    CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Collapsible, "false" );
    CORE::CDataNode* list = CFormLayout::AddWidgetNode( *window, "GridView", "Columns", "0,12 0,44", "1,-160 1,-60" );
    CFormLayout::SetWidgetNodeProperty( *list, EWidgetProperty_SelectionMode, "Cell" );
    CFormLayout::SetWidgetNodeProperty( *list, EWidgetProperty_CellEditing, "true" );
    CFormLayout::SetWidgetNodeProperty( *list, EWidgetProperty_FitColumns, "true" );
    CFormLayout::SetWidgetNodeProperty( *list, EWidgetProperty_ScrollBars, "true" );

    static const char* const buttonNames[] = { "MoveUp", "MoveDown", "Reset" };
    static const char* const buttonTexts[] = { "Move up", "Move down", "Reset to default" };
    for ( UInt32 i=0; i<3; ++i )
    {
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *window, "Button", buttonNames[ i ], "1,-136 0," + CORE::Int32ToString( 44 + 46 * (Int32) i ), "0,124 0,36" );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, buttonTexts[ i ] );
    }
    CORE::CDataNode* close = CFormLayout::AddWidgetNode( *window, "Button", "Close", "1,-136 1,-52", "0,124 0,38" );
    CFormLayout::SetWidgetNodeProperty( *close, EWidgetProperty_Text, "Close" );
    CFormLayout::SetWidgetNodeProperty( *close, EWidgetProperty_StyleClass, "Primary" );
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    CGridView* list = GetColumnGrid();
    if ( GUCEF_NULL != list )
        list->SetModel( m_modelPtr );
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* button = GetWidget( "MoveUp" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CGridColumnChooserForm::OnMoveUpActivated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "MoveDown" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CGridColumnChooserForm::OnMoveDownActivated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Reset" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CGridColumnChooserForm::OnResetActivated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Close" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CGridColumnChooserForm::OnCloseActivated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CGridColumnChooserForm::OnNavigation );
        SubscribeTo( root, CWidget::NavigationEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserForm::Open( CGridView& grid )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( m_grid != &grid )
    {
        if ( GUCEF_NULL != m_grid )
            UnsubscribeFrom( m_grid );
        m_grid = &grid;
        TEventCallback movedCallback( this, &CGridColumnChooserForm::OnGridColumnsChanged );
        SubscribeTo( m_grid, CGridView::ColumnMovedEvent, movedCallback );
        TEventCallback visibilityCallback( this, &CGridColumnChooserForm::OnGridColumnsChanged );
        SubscribeTo( m_grid, CGridView::ColumnVisibilityChangedEvent, visibilityCallback );
        TEventCallback resetCallback( this, &CGridColumnChooserForm::OnGridColumnsChanged );
        SubscribeTo( m_grid, CGridView::ColumnsResetEvent, resetCallback );
    }
    m_model->SetGrid( m_grid );
    CWidget* root = GetRootWidget();
    root->SetProperty( EWidgetProperty_Title, "Columns" );
    m_open = true;
    const bool shown = ShowModal();
    CGridView* list = GetColumnGrid();
    CGUIContext* context = GetGUIContext();
    if ( shown && GUCEF_NULL != list )
    {
        if ( m_model->GetRowCount() > 0 )
            list->SetCursor( 0, CGridColumnChooserModel::COLUMN_SHOWN );
        if ( GUCEF_NULL != context )
            context->SetFocus( list );
    }
    return shown;
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::Close( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    m_open = false;
    Hide();
    NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

CGridView*
CGridColumnChooserForm::GetGrid( void ) const
{GUCEF_TRACE;

    return m_grid;
}

/*-------------------------------------------------------------------------*/

CGridView*
CGridColumnChooserForm::GetColumnGrid( void )
{GUCEF_TRACE;

    return GetWidgetAs< CGridView >( "Columns" );
}

/*-------------------------------------------------------------------------*/

UInt32
CGridColumnChooserForm::GetRowCount( void ) const
{GUCEF_TRACE;

    return m_model->GetRowCount();
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnChooserForm::GetGridColumn( UInt32 row ) const
{GUCEF_TRACE;

    return m_model->GetGridColumn( row );
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserForm::SetColumnShown( UInt32 row, bool shown )
{GUCEF_TRACE;

    return m_model->SetCellValue( row, CGridColumnChooserModel::COLUMN_SHOWN, CORE::CVariant( shown ) ).IsAccepted();
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserForm::MoveRow( UInt32 row, bool up )
{GUCEF_TRACE;

    const Int32 column = m_model->GetGridColumn( row );
    if ( GUCEF_NULL == m_grid || column < 0 )
        return false;
    if ( up ? 0 == row : row + 1 >= m_model->GetRowCount() )
        return false;
    const UInt32 target = up ? row - 1 : row + 1;
    if ( !m_grid->MoveColumnInOrder( (UInt32) column, target ) )
        return false;
    CGridView* list = GetColumnGrid();
    if ( GUCEF_NULL != list )
        list->SetCursor( (Int32) target, list->GetCursorColumn() >= 0 ? list->GetCursorColumn() : (Int32) CGridColumnChooserModel::COLUMN_SHOWN );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::MoveCursorRow( bool up )
{GUCEF_TRACE;

    CGridView* list = GetColumnGrid();
    if ( GUCEF_NULL != list && list->GetCursorRow() >= 0 )
        MoveRow( (UInt32) list->GetCursorRow(), up );
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::ResetToDefault( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_grid )
        m_grid->ResetColumnsToDefault();
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnMoveUpActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MoveCursorRow( true );
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnMoveDownActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MoveCursorRow( false );
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnResetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ResetToDefault();
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CNavigationEventData* navigation = static_cast< CNavigationEventData* >( eventData );
    if ( NAVIGATION_CANCEL == navigation->direction && EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserForm::OnGridColumnsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_model->Refresh();
}

/*-------------------------------------------------------------------------*/

const CString&
CGridColumnChooserForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGridColumnChooserForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
