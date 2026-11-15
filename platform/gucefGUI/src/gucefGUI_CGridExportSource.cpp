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

#include "gucefGUI_CGridExportSource.h"

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

CGridExportSource::CGridExportSource( const CGridView& grid, bool rawValues )
    : CORE::CTSharedPtrCreator< CGridExportSource, MT::CMutex >( this )
    , m_grid( grid.HasModel() ? GUCEF_NULL : &grid )
    , m_model( grid.GetModel() )
    , m_columns()
    , m_titles()
    , m_rawValues( rawValues )
{GUCEF_TRACE;

    // the shown columns in the order shown
    const CGridColumnLayout& layout = grid.GetColumnLayout();
    const UInt32 positions = layout.GetPositionCount();
    m_columns.reserve( positions );
    m_titles.reserve( positions );
    for ( UInt32 p=0; p<positions; ++p )
    {
        const Int32 column = layout.GetColumn( (Int32) p );
        if ( column < 0 )
            continue;
        m_columns.push_back( column );
        m_titles.push_back( grid.GetColumnTitle( (UInt32) column ) );
    }
}

/*-------------------------------------------------------------------------*/

CGridExportSource::CGridExportSource( const TTableViewModelPtr& model, const TInt32Vector& columns, const TStringVector& titles, bool rawValues )
    : CORE::CTSharedPtrCreator< CGridExportSource, MT::CMutex >( this )
    , m_grid( GUCEF_NULL )
    , m_model( model )
    , m_columns()
    , m_titles()
    , m_rawValues( rawValues )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)columns.size(); ++i )
    {
        if ( columns[ i ] < 0 )
            continue;
        m_columns.push_back( columns[ i ] );
        if ( i < (UInt32) titles.size() )
            m_titles.push_back( titles[ i ] );
        else
        if ( !m_model.IsNULL() )
            m_titles.push_back( m_model->GetColumn( (UInt32) columns[ i ] ).GetDisplayTitle() );
        else
            m_titles.push_back( CString::Empty );
    }
}

/*-------------------------------------------------------------------------*/

CGridExportSource::~CGridExportSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGridExportSource::IsDetached( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_grid;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridExportSource::GetColumnCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_columns.size();
}

/*-------------------------------------------------------------------------*/

const CGridExportSource::TInt32Vector&
CGridExportSource::GetColumns( void ) const
{GUCEF_TRACE;

    return m_columns;
}

/*-------------------------------------------------------------------------*/

const CGridExportSource::TStringVector&
CGridExportSource::GetTitles( void ) const
{GUCEF_TRACE;

    return m_titles;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridExportSource::GetRowCount( void ) const
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
        return m_model->GetRowCount();
    return GUCEF_NULL != m_grid ? m_grid->GetRowCount() : 0;
}

/*-------------------------------------------------------------------------*/

void
CGridExportSource::PrepareRows( UInt32 firstRow, UInt32 rowCount )
{GUCEF_TRACE;

    if ( !m_model.IsNULL() && rowCount > 0 )
        m_model->PrepareRange( CTableViewRange( firstRow, rowCount, 0, m_model->GetColumnCount() ) );
}

/*-------------------------------------------------------------------------*/

bool
CGridExportSource::GetRowTexts( UInt32 row, TStringVector& cells ) const
{GUCEF_TRACE;

    const UInt32 columnCount = (UInt32) m_columns.size();
    cells.resize( columnCount );
    if ( !m_model.IsNULL() )
    {
        if ( row >= m_model->GetRowCount() )
            return false;
        for ( UInt32 i=0; i<columnCount; ++i )
        {
            const UInt32 column = (UInt32) m_columns[ i ];
            if ( m_rawValues )
            {
                CORE::CVariant value;
                if ( m_model->GetCellValue( row, column, value ) && !value.IsNULL() && !value.IsNIL() )
                    cells[ i ] = value.AsString();
                else
                    cells[ i ].Clear();
            }
            else
            {
                SCellContent content;
                if ( m_model->GetCellContent( row, column, content ) )
                {
                    // the marker of a status ("est.") is part of what the cell shows: the file says it where the screen does
                    if ( content.marker.IsNULLOrEmpty() )
                        cells[ i ] = content.text;
                    else
                    if ( content.markerLeading )
                        cells[ i ] = content.text.IsNULLOrEmpty() ? content.marker : content.marker + " " + content.text;
                    else
                        cells[ i ] = content.text.IsNULLOrEmpty() ? content.marker : content.text + " " + content.marker;
                }
                else
                    cells[ i ].Clear();
            }
        }
        return true;
    }
    if ( GUCEF_NULL == m_grid || row >= m_grid->GetRowCount() )
        return false;

    // a string list: the row is split once, the cells are its text
    const CGridView::TStringVector rowCells = m_grid->GetRowCells( row );
    for ( UInt32 i=0; i<columnCount; ++i )
    {
        const UInt32 column = (UInt32) m_columns[ i ];
        if ( column < (UInt32) rowCells.size() )
            cells[ i ] = rowCells[ column ];
        else
            cells[ i ].Clear();
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
