/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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


#include "gucefCORE_CDataTableRowCursor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableRowCursor::CDataTableRowCursor( const CDataTableBatchSourcePtr& source )
    : m_source( source )
    , m_batch()
    , m_singleBatch()
    , m_position( 0 )
    , m_row( 0 )
    , m_rowsVisited( 0 )
    , m_started( false )
    , m_ended( false )
    , m_lastStatus( DATATABLE_STATUS_OK )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableRowCursor::CDataTableRowCursor( const CDataTableBatchPtr& batch )
    : m_source()
    , m_batch()
    , m_singleBatch( batch )
    , m_position( 0 )
    , m_row( 0 )
    , m_rowsVisited( 0 )
    , m_started( false )
    , m_ended( false )
    , m_lastStatus( DATATABLE_STATUS_OK )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableRowCursor::~CDataTableRowCursor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableRowCursor::Next( void )
{GUCEF_TRACE;

    if ( m_ended )
        return DATATABLE_STATUS_END;

    // the next selected row of the current batch
    if ( m_started && !m_batch.IsNULL() && m_position + 1 < m_batch->GetSelectedRowCount() )
    {
        ++m_position;
        const UInt32* selection = m_batch->GetSelection();
        m_row = GUCEF_NULL == selection ? m_position : selection[ m_position ];
        ++m_rowsVisited;
        return DATATABLE_STATUS_OK;
    }
    m_started = true;

    // the first selected row of the next batch that has one
    while ( true )
    {
        CDataTableBatchPtr next;
        if ( !m_singleBatch.IsNULL() )
        {
            next.Swap( m_singleBatch );
        }
        else
        if ( !m_source.IsNULL() )
        {
            TDataTableStatus status = m_source->NextBatch( next );
            if ( DATATABLE_STATUS_OK != status )
            {
                m_batch.Reset();
                m_ended = true;
                m_lastStatus = status;
                return status;
            }
        }
        else
        {
            m_batch.Reset();
            m_ended = true;
            m_lastStatus = DATATABLE_STATUS_END;
            return DATATABLE_STATUS_END;
        }

        if ( next.IsNULL() || 0 == next->GetSelectedRowCount() )
            continue;

        m_batch.Swap( next );
        m_position = 0;
        const UInt32* selection = m_batch->GetSelection();
        m_row = GUCEF_NULL == selection ? 0 : selection[ 0 ];
        ++m_rowsVisited;
        return DATATABLE_STATUS_OK;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableRowCursor::GetValue( UInt32 projectedColumn, CVariant& out, bool linkIfPossible ) const
{

    if ( m_batch.IsNULL() )
    {
        out.Clear();
        return false;
    }
    return m_batch->GetValue( m_row, projectedColumn, out, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

const CDataTableBatchPtr&
CDataTableRowCursor::GetCurrentBatch( void ) const
{GUCEF_TRACE;

    return m_batch;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableRowCursor::GetCurrentRow( void ) const
{GUCEF_TRACE;

    return m_row;
}

/*-------------------------------------------------------------------------*/

UInt64
CDataTableRowCursor::GetRowsVisited( void ) const
{GUCEF_TRACE;

    return m_rowsVisited;
}

/*-------------------------------------------------------------------------*/

CString
CDataTableRowCursor::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    if ( !DataTableStatusIsError( m_lastStatus ) )
        return CString::Empty;
    if ( !m_source.IsNULL() )
        return m_source->GetLastErrorMessage();
    return DataTableStatusToString( m_lastStatus );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
