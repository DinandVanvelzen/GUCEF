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

#include <new>

#include "gucefCORE_CDataTableBufferPoolCore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const CDataTableColumnBatch g_emptyColumnBatch;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableBatchOrigin::CDataTableBatchOrigin( void )
    : fileOrdinal( 0 )
    , rowGroup( 0 )
    , firstRowInRowGroup( 0 )
{

}

/*-------------------------------------------------------------------------*/

bool
CDataTableBatchOrigin::operator==( const CDataTableBatchOrigin& other ) const
{

    return fileOrdinal == other.fileOrdinal && rowGroup == other.rowGroup && firstRowInRowGroup == other.firstRowInRowGroup;
}

/*-------------------------------------------------------------------------*/

CDataTableBatch::CDataTableBatch( CDataTableBufferPoolCore* core )
    : m_rowCount( 0 )
    , m_columnCount( 0 )
    , m_columns()
    , m_schemaColumnIndices()
    , m_schema()
    , m_selection( GUCEF_NULL )
    , m_selectedRowCount( 0 )
    , m_selectionOwner()
    , m_origin()
    , m_guaranteedPredicates( 0 )
    , m_refCount( 0 )
    , m_core( core )
    , m_nextFree( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableBatch::~CDataTableBatch()
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

const CDataTableColumnBatch&
CDataTableBatch::GetColumn( UInt32 projectedColumn ) const
{

    if ( projectedColumn < m_columnCount )
        return m_columns[ projectedColumn ];
    return g_emptyColumnBatch;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBatch::GetSchemaColumnIndex( UInt32 projectedColumn ) const
{

    if ( projectedColumn < m_columnCount )
        return m_schemaColumnIndices[ projectedColumn ];
    return GUCEF_UINT32MAX;
}

/*-------------------------------------------------------------------------*/

const CDataTableSchemaPtr&
CDataTableBatch::GetSchema( void ) const
{

    return m_schema;
}

/*-------------------------------------------------------------------------*/

const CDataTableBatchOrigin&
CDataTableBatch::GetOrigin( void ) const
{

    return m_origin;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableBatch::GetValue( UInt32 row, UInt32 projectedColumn, CVariant& out, bool linkIfPossible ) const
{

    if ( projectedColumn >= m_columnCount || row >= m_rowCount )
    {
        out.Clear();
        return false;
    }
    return m_columns[ projectedColumn ].GetValue( row, out, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBatch::GetReferenceCount( void ) const
{GUCEF_TRACE;

    return m_refCount.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::SetSchema( const CDataTableSchemaPtr& schema )
{GUCEF_TRACE;

    m_schema = schema;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::SetRowCount( UInt32 rowCount )
{GUCEF_TRACE;

    m_rowCount = rowCount;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableBatch::SetColumnCount( UInt32 projectedColumnCount )
{GUCEF_TRACE;

    try
    {
        if ( projectedColumnCount > (UInt32) m_columns.size() )
        {
            m_columns.resize( projectedColumnCount );
            m_schemaColumnIndices.resize( projectedColumnCount, GUCEF_UINT32MAX );
        }
    }
    catch ( const std::bad_alloc& )
    {
        return false;
    }

    // columns beyond the new count release what they held, the objects stay for reuse
    for ( UInt32 i=projectedColumnCount; i<m_columnCount; ++i )
    {
        m_columns[ i ].Reset();
        m_schemaColumnIndices[ i ] = GUCEF_UINT32MAX;
    }
    m_columnCount = projectedColumnCount;
    return true;
}

/*-------------------------------------------------------------------------*/

CDataTableColumnBatch&
CDataTableBatch::GetMutableColumn( UInt32 projectedColumn )
{GUCEF_TRACE;

    return m_columns[ projectedColumn ];
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::SetSchemaColumnIndex( UInt32 projectedColumn, UInt32 schemaColumnIndex )
{GUCEF_TRACE;

    if ( projectedColumn < m_columnCount )
        m_schemaColumnIndices[ projectedColumn ] = schemaColumnIndex;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::SetSelection( const UInt32* selection, UInt32 selectedRowCount, const CDataTableBufferPtr& owner )
{GUCEF_TRACE;

    m_selection = selection;
    m_selectedRowCount = GUCEF_NULL != selection ? selectedRowCount : 0;
    m_selectionOwner = GUCEF_NULL != selection ? owner : CDataTableBufferPtr();
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::SetOrigin( const CDataTableBatchOrigin& origin )
{GUCEF_TRACE;

    m_origin = origin;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::SetGuaranteedPredicates( UInt64 predicateMask )
{GUCEF_TRACE;

    m_guaranteedPredicates = predicateMask;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::Truncate( UInt32 rowCount )
{GUCEF_TRACE;

    if ( rowCount >= m_rowCount )
        return;

    for ( UInt32 i=0; i<m_columnCount; ++i )
        m_columns[ i ].Truncate( rowCount );
    m_rowCount = rowCount;

    if ( GUCEF_NULL != m_selection )
    {
        // the selection is ascending: keep the entries below the new row count
        UInt32 kept = m_selectedRowCount;
        while ( kept > 0 && m_selection[ kept - 1 ] >= rowCount )
            --kept;
        m_selectedRowCount = kept;
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::TruncateSelection( UInt32 selectedRowCount )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_selection )
    {
        Truncate( selectedRowCount );
        return;
    }
    if ( selectedRowCount >= m_selectedRowCount )
        return;

    m_selectedRowCount = selectedRowCount;
    Truncate( selectedRowCount > 0 ? m_selection[ selectedRowCount - 1 ] + 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::Reset( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_columnCount; ++i )
    {
        m_columns[ i ].Reset();
        m_schemaColumnIndices[ i ] = GUCEF_UINT32MAX;
    }
    m_columnCount = 0;
    m_rowCount = 0;
    m_selection = GUCEF_NULL;
    m_selectedRowCount = 0;
    m_selectionOwner.Reset();
    m_schema.Unlink();
    m_origin = CDataTableBatchOrigin();
    m_guaranteedPredicates = 0;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::AddReference( void )
{

    m_refCount.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatch::ReleaseReference( void )
{

    if ( 1 == m_refCount.FetchSub( 1, MT::MEMORDER_ACQ_REL ) )
    {
        // the columns release their buffers (and dictionaries), then the object goes back to the free list
        Reset();
        m_core->ReturnBatch( this );
    }
}

/*-------------------------------------------------------------------------*/

CDataTableBatchPtr::CDataTableBatchPtr( void )
    : m_batch( GUCEF_NULL )
{

}

/*-------------------------------------------------------------------------*/

CDataTableBatchPtr::CDataTableBatchPtr( const CDataTableBatchPtr& src )
    : m_batch( src.m_batch )
{

    if ( GUCEF_NULL != m_batch )
        m_batch->AddReference();
}

/*-------------------------------------------------------------------------*/

CDataTableBatchPtr::~CDataTableBatchPtr()
{

    Reset();
}

/*-------------------------------------------------------------------------*/

CDataTableBatchPtr&
CDataTableBatchPtr::operator=( const CDataTableBatchPtr& src )
{

    if ( src.m_batch != m_batch )
    {
        CDataTableBatch* previous = m_batch;
        m_batch = src.m_batch;
        if ( GUCEF_NULL != m_batch )
            m_batch->AddReference();
        if ( GUCEF_NULL != previous )
            previous->ReleaseReference();
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatchPtr::Reset( void )
{

    if ( GUCEF_NULL != m_batch )
    {
        CDataTableBatch* previous = m_batch;
        m_batch = GUCEF_NULL;
        previous->ReleaseReference();
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatchPtr::Swap( CDataTableBatchPtr& other )
{

    CDataTableBatch* mine = m_batch;
    m_batch = other.m_batch;
    other.m_batch = mine;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatchPtr::Adopt( CDataTableBatch* batch )
{

    Reset();
    m_batch = batch;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
