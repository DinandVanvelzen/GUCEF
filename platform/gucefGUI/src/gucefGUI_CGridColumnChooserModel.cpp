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

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#include "gucefGUI_CGridColumnChooserModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridColumnChooserModel::CGridColumnChooserModel( void )
    : CITableViewModel()
    , m_grid( GUCEF_NULL )
    , m_order()
{GUCEF_TRACE;

    m_columns[ COLUMN_SHOWN ] = CTableViewColumn( "Shown", CORE::DATATABLE_TYPE_BOOL, 64, false );
    m_columns[ COLUMN_TITLE ] = CTableViewColumn( "Column", CORE::DATATABLE_TYPE_UTF8, 0, true );
    m_columns[ COLUMN_NOTE ] = CTableViewColumn( "Note", CORE::DATATABLE_TYPE_UTF8, 140, true );
}

/*-------------------------------------------------------------------------*/

CGridColumnChooserModel::~CGridColumnChooserModel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserModel::SetGrid( CGridView* grid )
{GUCEF_TRACE;

    m_grid = grid;
    Refresh();
}

/*-------------------------------------------------------------------------*/

CGridView*
CGridColumnChooserModel::GetGrid( void ) const
{GUCEF_TRACE;

    return m_grid;
}

/*-------------------------------------------------------------------------*/

void
CGridColumnChooserModel::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_grid )
        m_order = m_grid->GetColumnOrder();
    else
        m_order.clear();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnChooserModel::GetGridColumn( UInt32 row ) const
{GUCEF_TRACE;

    return row < (UInt32) m_order.size() ? m_order[ row ] : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnChooserModel::FindRow( Int32 gridColumn ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_order.size(); ++i )
        if ( m_order[ i ] == gridColumn )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridColumnChooserModel::GetRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_order.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CGridColumnChooserModel::GetColumnCount( void ) const
{GUCEF_TRACE;

    return (UInt32) COLUMN_COUNT;
}

/*-------------------------------------------------------------------------*/

const CTableViewColumn&
CGridColumnChooserModel::GetColumn( UInt32 column ) const
{GUCEF_TRACE;

    return m_columns[ column < (UInt32) COLUMN_COUNT ? column : (UInt32) COLUMN_TITLE ];
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserModel::GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const
{GUCEF_TRACE;

    const Int32 gridColumn = GetGridColumn( row );
    if ( gridColumn < 0 || GUCEF_NULL == m_grid || column >= (UInt32) COLUMN_COUNT )
        return false;
    switch ( column )
    {
        case COLUMN_SHOWN:
        {
            value = !m_grid->IsColumnHidden( (UInt32) gridColumn );
            return true;
        }
        case COLUMN_TITLE:
        {
            const CString title = m_grid->GetColumnTitle( (UInt32) gridColumn );
            value = title.IsNULLOrEmpty() ? "Column " + CORE::UInt32ToString( (UInt32) gridColumn + 1 ) : title;
            return true;
        }
        default:
        {
            if ( m_grid->IsColumnRequired( (UInt32) gridColumn ) )
                value = CString( "Always shown" );
            else
            if ( m_grid->IsColumnHiddenByDefault( (UInt32) gridColumn ) )
                value = CString( "Hidden by default" );
            else
                value = CString::Empty;
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserModel::GetRowKey( UInt32 row, CORE::CVariant& key ) const
{GUCEF_TRACE;

    const Int32 gridColumn = GetGridColumn( row );
    if ( gridColumn < 0 )
        return false;
    key = gridColumn;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserModel::FindRowByKey( const CORE::CVariant& key, UInt32& row ) const
{GUCEF_TRACE;

    const Int32 found = FindRow( key.AsInt32( -1 ) );
    if ( found < 0 )
        return false;
    row = (UInt32) found;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnChooserModel::IsCellReadOnly( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    const Int32 gridColumn = GetGridColumn( row );
    if ( COLUMN_SHOWN != column || gridColumn < 0 || GUCEF_NULL == m_grid )
        return true;
    // a column that is shown and cannot be hidden keeps its tick
    return !m_grid->IsColumnHidden( (UInt32) gridColumn ) && !m_grid->CanHideColumn( (UInt32) gridColumn );
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CGridColumnChooserModel::ValidateCellEdit( UInt32 row, UInt32 column, const CORE::CVariant& value ) const
{GUCEF_TRACE;

    if ( IsCellReadOnly( row, column ) )
    {
        const Int32 gridColumn = GetGridColumn( row );
        if ( gridColumn >= 0 && GUCEF_NULL != m_grid && m_grid->IsColumnRequired( (UInt32) gridColumn ) )
            return CTableEditResult::Refused( "This column is always shown" );
        if ( COLUMN_SHOWN == column )
            return CTableEditResult::Refused( "At least one column is shown" );
        return CTableEditResult::ReadOnly();
    }
    return CTableEditResult::Accepted();
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CGridColumnChooserModel::SetCellValue( UInt32 row, UInt32 column, const CORE::CVariant& value )
{GUCEF_TRACE;

    const CTableEditResult validation = ValidateCellEdit( row, column, value );
    if ( !validation.IsAccepted() )
        return validation;
    const Int32 gridColumn = GetGridColumn( row );
    if ( !m_grid->SetColumnHidden( (UInt32) gridColumn, !value.AsBool( true ) ) )
        return CTableEditResult::Refused( "This column cannot be hidden" );
    TTableViewRangeEventData changed( CTableViewRange( row, 1, 0, (UInt32) COLUMN_COUNT ) );
    NotifyObservers( CellsChangedEvent, &changed );
    return CTableEditResult::Accepted();
}

/*-------------------------------------------------------------------------*/

const CString&
CGridColumnChooserModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGridColumnChooserModel";
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
