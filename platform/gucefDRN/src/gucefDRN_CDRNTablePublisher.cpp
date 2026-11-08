/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#include "gucefDRN_CDRNTablePublisherUpdateScope.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H ? */

#include "gucefDRN_CDRNTablePublisher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CDRNTablePublisher::SchemaKey = "#schema";
const char* CDRNTablePublisher::MetaKey = "#meta";

/*-------------------------------------------------------------------------*/

CDRNTablePublisher::CDRNTablePublisher( CDRNDataGrid& grid, const CDRNTablePublisherSettings& settings )
    : m_grid( &grid )
    , m_settings( settings )
    , m_valid( false )
    , m_schema()
    , m_haveSchema( false )
    , m_dataSchema()
    , m_checker( GUCEF_NULL )
    , m_meta()
    , m_liveKeys()
    , m_changedKeys()
    , m_refusedKeys()
    , m_removedInUpdate( 0 )
    , m_metaChanged( false )
    , m_updateDepth( 0 )
    , m_updateThreadId( 0 )
    , m_rowPrefixSize( 0 )
    , m_stream()
    , m_existing()
    , m_oversizeRows( 0 )
    , m_typeMismatches( 0 )
    , m_reservedKeys( 0 )
{GUCEF_TRACE;

    m_valid = grid.IsOrigin() && grid.GetSettings().reliable && 0 == grid.GetKeyCount();
    if ( !m_valid )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher(" << grid.GetName() << "): the grid must be local, reliable and empty" );

    // the size of version and revision in front of the cells, to compare the cells of a row with what the grid holds
    CORE::CVariantStream prefix;
    prefix << RowEncodingVersion << (UInt64) 0;
    m_rowPrefixSize = prefix.GetDataSize();
}

/*-------------------------------------------------------------------------*/

CDRNTablePublisher::~CDRNTablePublisher()
{GUCEF_TRACE;

    GUCEF_DELETE m_checker;
    m_checker = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::IsValid( void ) const
{GUCEF_TRACE;

    return m_valid;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::IsInUpdateOnThisThread( void ) const
{GUCEF_TRACE;

    // set and cleared by the thread itself under the node lock: another thread's update never matches
    return m_updateThreadId.Load() == MT::GetCurrentTaskID();
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CDRNTablePublisher::GetNodeLock( void ) const
{GUCEF_TRACE;

    return m_grid->GetSession().GetNode().GetLock();
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::IsReservedKey( const CORE::CVariant& key )
{GUCEF_TRACE;

    if ( !key.IsString() )
        return false;
    CORE::CString text = key.AsString();
    return !text.IsNULLOrEmpty() && '#' == text[ 0 ];
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::WriteValue( const CORE::CVariant& key, const CORE::CVariantStream& stream )
{GUCEF_TRACE;

    CORE::CVariant value;
    value.LinkTo( stream.GetBuffer().GetConstBufferPtr(), stream.GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
    return m_grid->SetValue( key, value );
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::SetSchema( const CDRNPublishedTableSchema& schema )
{GUCEF_TRACE;

    CORE::CString error;
    if ( !m_valid || !schema.IsValid( error ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: the schema is not valid: " << error );
        return false;
    }
    CORE::CDataTableSchema dataSchema;
    if ( !schema.ToDataTableSchema( dataSchema ) )
        return false;
    MT::CScopeMutex lock( GetNodeLock() );
    m_stream.Clear();
    schema.Encode( m_stream );
    if ( m_stream.GetDataSize() > m_settings.maxValueBytes )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: the schema needs " << m_stream.GetDataSize() << " bytes, more than maxValueBytes" );
        return false;
    }
    m_schema = schema;
    m_haveSchema = true;
    m_dataSchema = CORE::CreateDataTableSchemaPtr( dataSchema );
    GUCEF_DELETE m_checker;
    m_checker = GUCEF_NEW CORE::CDataTableBatchBuilder( m_dataSchema, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
    m_metaChanged = true;

    // first written, so #schema and #meta hold the lowest key indices
    if ( !WriteValue( CORE::CVariant( SchemaKey ), m_stream ) )
        return false;
    if ( m_meta.pageRevision == 0 && !IsInUpdateOnThisThread() )
    {
        m_stream.Clear();
        m_meta.Encode( m_stream );
        WriteValue( CORE::CVariant( MetaKey ), m_stream );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CDRNPublishedTableSchema&
CDRNTablePublisher::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::BeginUpdate( void )
{GUCEF_TRACE;

    if ( !m_valid || !m_haveSchema )
        return false;
    // what changed and is not published yet (a scalar or the schema set between updates) goes with this update. The
    // node lock is held until the update ends: an update of another thread waits for this one
    GetNodeLock().Lock();
    if ( 0 == m_updateDepth )
        m_updateThreadId.Store( MT::GetCurrentTaskID() );
    ++m_updateDepth;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::EndUpdate( void )
{GUCEF_TRACE;

    if ( !IsInUpdateOnThisThread() )
        return false;
    bool success = true;
    if ( 1 == m_updateDepth && ( !m_changedKeys.empty() || 0 != m_removedInUpdate || m_metaChanged ) )
    {
        // #meta last: the replica counts the rows of this revision against it
        ++m_meta.pageRevision;
        m_meta.rowCount = (UInt32) m_liveKeys.size();
        m_meta.changedRowCount = (UInt32) m_changedKeys.size() + m_removedInUpdate;
        m_stream.Clear();
        m_meta.Encode( m_stream );
        m_stream << m_removedInUpdate;          // appended: the removals of this revision, for the count of the replica
        success = WriteValue( CORE::CVariant( MetaKey ), m_stream );
        if ( success )
        {
            m_changedKeys.clear();
            m_removedInUpdate = 0;
            m_metaChanged = false;
        }
        else
        {
            // the rows written carry this revision: the next update publishes it again
            --m_meta.pageRevision;
        }
    }
    --m_updateDepth;
    if ( 0 == m_updateDepth )
        m_updateThreadId.Store( 0 );
    GetNodeLock().Unlock();
    return success;
}

/*-------------------------------------------------------------------------*/

void
CDRNTablePublisher::NoteRefusal( const CORE::CVariant& key, const char* why )
{GUCEF_TRACE;

    if ( m_refusedKeys.find( key ) != m_refusedKeys.end() )
        return;
    m_refusedKeys.insert( key );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher(" << m_grid->GetName() << "): row " << key.AsString() << " refused: " << why );
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::CheckCells( const CORE::CVariant& key, const CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    // the values in the order of the data table schema (key first), checked as the batch builder of a replica will
    const UInt32 columnCount = (UInt32) m_schema.columns.size();
    if ( cells.size() + 1 != columnCount )
        return false;
    if ( !m_settings.checkRows || GUCEF_NULL == m_checker )
        return true;
    m_checker->Reset();
    CORE::CVariant::VariantVector ordered( columnCount );
    ordered[ 0 ] = key;
    UInt32 cell = 0;
    for ( UInt32 c=0; c<columnCount; ++c )
    {
        if ( c == m_schema.keyColumn )
            continue;
        ordered[ m_schema.ToDataTableColumnIndex( c ) ].LinkTo( cells[ cell++ ] );
    }
    bool fits = m_checker->AppendRow( &ordered[ 0 ], columnCount );
    m_checker->Reset();
    return fits;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::SetRow( const CORE::CVariant& key, const CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    if ( !m_valid || !m_haveSchema )
        return false;
    if ( !IsInUpdateOnThisThread() )
    {
        // outside an update: an update of its own, so that #meta counts the row and replicas stay consistent
        CDRNTablePublisherUpdateScope update( *this );
        if ( !update.IsOpen() )
            return false;
        bool set = SetRow( key, cells );
        return update.EarlyEnd() && set;
    }
    if ( IsReservedKey( key ) )
    {
        ++m_reservedKeys;
        NoteRefusal( key, "text keys starting with '#' are reserved" );
        return false;
    }
    if ( !CheckCells( key, cells ) )
    {
        ++m_typeMismatches;
        NoteRefusal( key, "a cell does not fit its column" );
        return false;
    }

    m_stream.Clear();
    m_stream << RowEncodingVersion << ( m_meta.pageRevision + 1 );
    for ( UInt32 i=0; i<cells.size(); ++i )
    {
        // a null cell is a false presence flag, a value a true one and the value
        bool present = cells[ i ].IsInitialized() && !cells[ i ].IsNULL();
        m_stream << present;
        if ( present )
            m_stream << cells[ i ];
    }
    if ( m_stream.GetDataSize() > m_settings.maxValueBytes )
    {
        ++m_oversizeRows;
        NoteRefusal( key, "the row is larger than maxValueBytes" );
        return false;
    }

    MT::CScopeMutex lock( m_grid->GetSession().GetNode().GetLock() );
    m_liveKeys.insert( key );

    // the same cells as the grid holds: nothing is written, nothing travels
    if ( m_grid->GetValue( key, m_existing ) && m_existing.ByteSize( false ) == m_stream.GetDataSize() &&
         0 == memcmp( static_cast< const UInt8* >( m_existing.AsVoidPtr() ) + m_rowPrefixSize,
                      static_cast< const UInt8* >( m_stream.GetBuffer().GetConstBufferPtr() ) + m_rowPrefixSize,
                      m_stream.GetDataSize() - m_rowPrefixSize ) )
        return true;

    if ( !WriteValue( key, m_stream ) )
        return false;
    m_changedKeys.insert( key );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::RemoveRow( const CORE::CVariant& key )
{GUCEF_TRACE;

    if ( !m_valid )
        return false;
    if ( !IsInUpdateOnThisThread() )
    {
        CDRNTablePublisherUpdateScope update( *this );
        if ( !update.IsOpen() )
            return false;
        bool removed = RemoveRow( key );
        return update.EarlyEnd() && removed;
    }
    MT::CScopeMutex lock( m_grid->GetSession().GetNode().GetLock() );
    TKeySet::iterator i = m_liveKeys.find( key );
    if ( i == m_liveKeys.end() )
        return false;
    m_liveKeys.erase( i );
    m_changedKeys.erase( key );
    if ( !m_grid->RemoveKey( key ) )
        return false;
    ++m_removedInUpdate;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::PublishRows( const TKeyedRows& rows )
{GUCEF_TRACE;

    if ( !m_valid || !m_haveSchema )
        return false;
    if ( !IsInUpdateOnThisThread() )
    {
        // the whole row set is one revision
        CDRNTablePublisherUpdateScope update( *this );
        if ( !update.IsOpen() )
            return false;
        bool published = PublishRows( rows );
        return update.EarlyEnd() && published;
    }
    bool allSet = true;
    TKeySet given;
    for ( UInt32 i=0; i<rows.size(); ++i )
    {
        given.insert( rows[ i ].first );
        allSet = SetRow( rows[ i ].first, rows[ i ].second ) && allSet;
    }
    TKeySet gone;
    for ( TKeySet::const_iterator i=m_liveKeys.begin(); i!=m_liveKeys.end(); ++i )
        if ( given.find( *i ) == given.end() )
            gone.insert( *i );
    for ( TKeySet::const_iterator i=gone.begin(); i!=gone.end(); ++i )
        RemoveRow( *i );
    return allSet;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisher::PublishFrom( CORE::CIDataTable& table, const CORE::CDataTableQuery& query )
{GUCEF_TRACE;

    if ( !m_valid || !m_haveSchema || m_schema.columns.empty() || m_schema.keyColumn >= m_schema.columns.size() )
        return false;

    // The node lock is held while the table is scanned inside an update (BeginUpdate()) and on a thread that works on
    // the node (a handler of a pulse of the node's pulse generator, CDRNNode::IsWorkingOnThisThread()), and the scan
    // subscribes to the pulse generator of the table, whose pulses may take the node lock: there that is only done on the
    // thread that pulses it, where no pulse of it can wait for this thread (gucefDRN design 9.1). Elsewhere the rows are
    // read before the update. An update that another thread has open is not this thread's: the update of its own waits
    // for that one
    const bool inUpdate = IsInUpdateOnThisThread();
    if ( inUpdate || m_grid->GetSession().GetNode().IsWorkingOnThisThread() )
    {
        CORE::PulseGeneratorPtr tablePulseGenerator = table.GetPulseGenerator();
        if ( tablePulseGenerator.IsNULL() )
            tablePulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        if ( tablePulseGenerator->GetPulseDriverThreadId() != MT::GetCurrentTaskID() )
        {
            if ( inUpdate )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: Table \"" << table.GetName() << "\" is scanned inside an update only on the thread of "
                                 "its pulse generator: the update holds the node lock; publish it outside the update" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: Table \"" << table.GetName() << "\" is scanned by a thread that works on the node "
                                 "only on the thread of its pulse generator: that thread holds the node lock; publish it from another thread" );
            }
            return false;
        }
    }

    CORE::CDataTableScanPtr scan;
    CORE::TDataTableStatus status = table.Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status || scan.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: Cannot scan table \"" << table.GetName() << "\": " << table.GetLastErrorMessage() );
        return false;
    }

    // the rows are read first: the scan may take its time and the update holds the node lock
    const UInt32 columnCount = (UInt32) m_schema.columns.size();
    TKeyedRows rows;
    GUCEF::vector< UInt32 > projected( columnCount, 0 );
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) && !batch.IsNULL() )
    {
        // where each published column is in the batch, by name
        const CORE::CDataTableSchemaPtr& batchSchema = batch->GetSchema();
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            bool found = false;
            for ( UInt32 p=0; p<batch->GetColumnCount() && !batchSchema.IsNULL(); ++p )
            {
                if ( batchSchema->GetColumn( batch->GetSchemaColumnIndex( p ) ).name == m_schema.columns[ c ].def.name )
                {
                    projected[ c ] = p;
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: The scan of table \"" << table.GetName() << "\" has no column \"" <<
                                 m_schema.columns[ c ].def.name << "\" of the published schema" );
                return false;
            }
        }

        const UInt32* selection = batch->GetSelection();
        const UInt32 selected = batch->GetSelectedRowCount();
        for ( UInt32 s=0; s<selected; ++s )
        {
            UInt32 r = GUCEF_NULL != selection ? selection[ s ] : s;
            rows.push_back( TKeyedRow() );
            TKeyedRow& row = rows.back();
            row.second.reserve( columnCount - 1 );
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                CORE::CVariant value;
                batch->GetValue( r, projected[ c ], value, false );
                if ( c == m_schema.keyColumn )
                    row.first = value;
                else
                    row.second.push_back( value );
            }
        }
    }
    if ( CORE::DATATABLE_STATUS_END != status && CORE::DATATABLE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTablePublisher: The scan of table \"" << table.GetName() << "\" failed: " << scan->GetLastErrorMessage() );
        return false;
    }

    CDRNTablePublisherUpdateScope update( *this );
    return update.IsOpen() && PublishRows( rows );
}

/*-------------------------------------------------------------------------*/

void
CDRNTablePublisher::SetScalar( const CORE::CString& name, const CORE::CVariant& value )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    if ( m_meta.scalars.HasKey( name ) && m_meta.scalars.GetValue( name ) == value )
        return;
    m_meta.scalars.Set( name, value );
    m_metaChanged = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNTablePublisher::SetSourceVersion( UInt64 sourceVersion )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    if ( m_meta.sourceVersion == sourceVersion )
        return;
    m_meta.sourceVersion = sourceVersion;
    m_metaChanged = true;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNTablePublisher::GetPageRevision( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    return m_meta.pageRevision;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNTablePublisher::GetRowCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    return (UInt32) m_liveKeys.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNTablePublisher::GetOversizeRowCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    return m_oversizeRows;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNTablePublisher::GetTypeMismatchCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    return m_typeMismatches;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNTablePublisher::GetReservedKeyCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetNodeLock() );
    return m_reservedKeys;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
