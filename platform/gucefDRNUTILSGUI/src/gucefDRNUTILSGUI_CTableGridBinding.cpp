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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#include "gucefDRN_CDRNPublishedTable.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLE_H ? */

#include "gucefDRNUTILSGUI_CTableGridBinding.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CTableGridBinding::ModelChangedEvent = "GUCEF::DRNUTILSGUI::CTableGridBinding::ModelChangedEvent";
const CORE::CEvent CTableGridBinding::SelectionChangedEvent = "GUCEF::DRNUTILSGUI::CTableGridBinding::SelectionChangedEvent";

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::RegisterEvents( void )
{GUCEF_TRACE;

    ModelChangedEvent.Initialize();
    SelectionChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTableGridBinding::CTableGridBinding( void )
    : CORE::CObservingNotifier()
    , m_grid( GUCEF_NULL )
    , m_model()
    , m_timeModel( GUCEF_NULL )
    , m_keyColumn()
    , m_table()
    , m_specs()
    , m_timeDisplay()
    , m_hasSort( false )
    , m_sortColumn()
    , m_sortAscending( true )
    , m_filter()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTableGridBinding::~CTableGridBinding()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::Bind( GUI::CGridView* grid                    ,
                         const CORE::CIDataTablePtr& table       ,
                         const CORE::CString& keyColumn          ,
                         const SGridColumnSpec* specs            ,
                         UInt32 specCount                        ,
                         const CTimeDisplaySettings& timeDisplay )
{GUCEF_TRACE;

    Unbind();
    m_grid = grid;
    m_keyColumn = keyColumn;
    m_table = table;
    m_timeDisplay = timeDisplay;
    m_specs.clear();
    for ( UInt32 i=0; GUCEF_NULL != specs && i<specCount; ++i )
        m_specs.push_back( specs[ i ] );
    if ( GUCEF_NULL == m_grid )
        return false;

    TEventCallback selectionCallback( this, &CTableGridBinding::OnGridSelectionChanged );
    SubscribeTo( m_grid, GUI::CGridView::SelectionChangedEvent, selectionCallback );
    m_grid->SetBoolProperty( GUI::EWidgetProperty_Sortable, true );
    m_grid->SetBoolProperty( GUI::EWidgetProperty_ResizableColumns, true );

    if ( m_table.IsNULL() )
    {
        m_grid->SetModel( GUI::TTableViewModelPtr() );
        return true;
    }

    // a table whose schema changes (a DRN replica: its schema may arrive after the binding) gets a new model
    TEventCallback shapeCallback( this, &CTableGridBinding::OnTableShapeChanged );
    SubscribeTo( m_table.GetPointerAlways(), DRN::CDRNPublishedTable::SchemaChangedEvent, shapeCallback );
    if ( BuildModel() )
        return true;

    // nothing to show yet: the rows that arrive (they follow the schema) bring the columns
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTableGridBinding:Bind: The table " << m_table->GetName() << " has no columns that can be shown yet; it is shown once its schema arrives" );
    TEventCallback rowsCallback( this, &CTableGridBinding::OnTableShapeChanged );
    SubscribeTo( m_table.GetPointerAlways(), CORE::CIDataTable::RowsChangedEvent, rowsCallback );
    m_grid->SetModel( GUI::TTableViewModelPtr() );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::BuildModel( void )
{GUCEF_TRACE;

    if ( m_table.IsNULL() || GUCEF_NULL == m_grid )
        return false;
    if ( GUCEF_NULL != m_timeModel )
        UnsubscribeFrom( m_timeModel );
    CTimeDisplayTableModel* model = GUCEF_NEW CTimeDisplayTableModel( m_table, m_specs.empty() ? GUCEF_NULL : &m_specs[ 0 ], (UInt32) m_specs.size(), m_timeDisplay );
    GUI::TTableViewModelPtr modelPtr( model );
    if ( !model->IsValid() || 0 == model->GetColumnCount() )
    {
        m_grid->SetModel( GUI::TTableViewModelPtr() );
        m_model = GUI::TTableViewModelPtr();
        m_timeModel = GUCEF_NULL;
        return false;
    }
    m_model = modelPtr;
    m_timeModel = model;
    TEventCallback resetCallback( this, &CTableGridBinding::OnModelReset );
    SubscribeTo( model, GUI::CITableViewModel::ModelResetEvent, resetCallback );
    if ( !m_filter.empty() )
        m_timeModel->SetFilter( m_filter );
    if ( m_hasSort )
    {
        const UInt32 column = m_timeModel->FindShownColumn( m_sortColumn );
        if ( GUCEF_UINT32MAX != column && m_timeModel->IsSortSupported() )
            m_timeModel->SortByColumn( column, m_sortAscending );
    }
    m_grid->SetModel( m_model );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::OnTableShapeChanged( CORE::CNotifier* notifier   ,
                                        const CORE::CEvent& eventId ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a new schema, or the first rows of a table that had no columns to show: a model that fits it
    const bool waiting = GUCEF_NULL == m_timeModel;
    if ( !waiting && CORE::CIDataTable::RowsChangedEvent == eventId )
        return;
    if ( BuildModel() )
    {
        if ( waiting && !m_table.IsNULL() )
            UnsubscribeFrom( m_table.GetPointerAlways(), CORE::CIDataTable::RowsChangedEvent );
        NotifyObservers( ModelChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::Unbind( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_grid )
        m_grid->SetModel( GUI::TTableViewModelPtr() );
    m_model = GUI::TTableViewModelPtr();
    m_timeModel = GUCEF_NULL;
    m_grid = GUCEF_NULL;
    m_table.Unlink();
    m_hasSort = false;
    m_sortColumn.Clear();
    m_filter.clear();
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::IsBound( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_timeModel;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTableGridBinding::GetKeyColumn( void ) const
{GUCEF_TRACE;

    return m_keyColumn;
}

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::SetTimeDisplay( const CTimeDisplaySettings& timeDisplay )
{GUCEF_TRACE;

    m_timeDisplay = timeDisplay;
    if ( GUCEF_NULL != m_timeModel )
        m_timeModel->SetTimeDisplay( timeDisplay );
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::SetFilter( const CORE::CDataTableQuery::TPredicateVector& predicates )
{GUCEF_TRACE;

    m_filter = predicates;
    if ( GUCEF_NULL == m_timeModel )
        return false;
    if ( predicates.empty() )
    {
        m_timeModel->ClearFilter();
        return true;
    }
    return m_timeModel->SetFilter( predicates );
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::SortBy( const CORE::CString& columnName, bool ascending )
{GUCEF_TRACE;

    m_hasSort = true;
    m_sortColumn = columnName;
    m_sortAscending = ascending;
    if ( GUCEF_NULL == m_timeModel )
        return false;
    const UInt32 column = m_timeModel->FindShownColumn( columnName );
    if ( GUCEF_UINT32MAX == column || !m_timeModel->IsSortSupported() )
        return false;
    return m_timeModel->SortByColumn( column, ascending );
}

/*-------------------------------------------------------------------------*/

UInt32
CTableGridBinding::GetRowCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_timeModel ? m_timeModel->GetRowCount() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::GetCellText( UInt32 row, const CORE::CString& columnName, CORE::CString& text ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_timeModel )
        return false;
    const UInt32 column = m_timeModel->FindShownColumn( columnName );
    if ( GUCEF_UINT32MAX == column || row >= m_timeModel->GetRowCount() )
        return false;
    text = m_timeModel->GetCellText( row, column );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::GetCellValue( UInt32 row, const CORE::CString& columnName, CORE::CVariant& value ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_timeModel )
        return false;
    const UInt32 column = m_timeModel->FindShownColumn( columnName );
    if ( GUCEF_UINT32MAX == column || row >= m_timeModel->GetRowCount() )
        return false;
    return m_timeModel->GetCellValue( row, column, value );
}

/*-------------------------------------------------------------------------*/

Int32
CTableGridBinding::GetSelectedRow( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_grid || GUCEF_NULL == m_timeModel )
        return -1;
    const Int32 row = m_grid->GetSelectedRow();
    return row >= 0 && (UInt32) row < m_timeModel->GetRowCount() ? row : -1;
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::GetSelectedCellValue( const CORE::CString& columnName, CORE::CVariant& value ) const
{GUCEF_TRACE;

    const Int32 row = GetSelectedRow();
    return row >= 0 && GetCellValue( (UInt32) row, columnName, value );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTableGridBinding::GetSelectedKeyText( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !GetSelectedCellValue( m_keyColumn, value ) )
        return CORE::CString::Empty;
    return value.AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

bool
CTableGridBinding::SelectKeyText( const CORE::CString& keyText )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_grid || GUCEF_NULL == m_timeModel )
        return false;
    const UInt32 keyIndex = m_timeModel->FindShownColumn( m_keyColumn );
    if ( GUCEF_UINT32MAX == keyIndex )
        return false;

    const UInt32 count = m_timeModel->GetRowCount();
    for ( UInt32 row=0; row<count; ++row )
    {
        CORE::CVariant value;
        if ( m_timeModel->GetCellValue( row, keyIndex, value ) && value.AsString( CORE::CString::Empty, false ) == keyText )
        {
            m_grid->SetSelectedRow( (Int32) row );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::ClearSelection( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_grid )
        m_grid->SetSelectedRow( -1 );
}

/*-------------------------------------------------------------------------*/

const CTimeDisplayTableModel*
CTableGridBinding::GetModel( void ) const
{GUCEF_TRACE;

    return m_timeModel;
}

/*-------------------------------------------------------------------------*/

GUI::CGridView*
CTableGridBinding::GetGrid( void ) const
{GUCEF_TRACE;

    return m_grid;
}

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::OnModelReset( CORE::CNotifier* notifier   ,
                                 const CORE::CEvent& eventId ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( ModelChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTableGridBinding::OnGridSelectionChanged( CORE::CNotifier* notifier   ,
                                           const CORE::CEvent& eventId ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( SelectionChangedEvent );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTableGridBinding::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString typeName = "GUCEF::DRNUTILSGUI::CTableGridBinding";
    return typeName;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
