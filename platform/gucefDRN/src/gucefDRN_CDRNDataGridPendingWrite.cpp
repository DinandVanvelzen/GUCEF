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

#include <algorithm>

#include "gucefDRN_DRNProtocol.h"
#include "gucefDRN_CDRNDataGridPendingWrite.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
AnswerComesBefore( const CDRNDataGridPendingWrite::CAnswer& a, const CDRNDataGridPendingWrite::CAnswer& b )
{
    return DRNIsNewerSequence( b.proposalId, a.proposalId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRowChange::CDRNRowChange( void )
    : key()
    , value()
    , remove( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGridPendingWrite::CAnswer::CAnswer( void )
    : proposalId( 0 )
    , accepted( false )
    , voteOutcome( 0 )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGridPendingWrite::CDRNDataGridPendingWrite( const TDRNRowChangeVector& changes, CORE::TDataTableWriteMode mode, UInt32 timeoutMs )
    : m_changes( changes )
    , m_mode( mode )
    , m_sent()
    , m_proposedRows()
    , m_answers()
    , m_source()
    , m_timeoutMs( timeoutMs )
    , m_deadline( 0 )
    , m_started( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::TDataTableWriteMode
CDRNDataGridPendingWrite::GetWriteMode( void ) const
{GUCEF_TRACE;

    return m_mode;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridPendingWrite::GetFuture( void ) const
{GUCEF_TRACE;

    return m_source.GetFuture();
}

/*-------------------------------------------------------------------------*/

const TDRNRowChangeVector&
CDRNDataGridPendingWrite::GetChanges( void ) const
{GUCEF_TRACE;

    return m_changes;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< UInt32 >&
CDRNDataGridPendingWrite::GetProposedRows( void ) const
{GUCEF_TRACE;

    return m_proposedRows;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridPendingWrite::SetProposedRows( const GUCEF::vector< UInt32 >& proposedRows )
{GUCEF_TRACE;

    m_proposedRows = proposedRows;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableWriteResult&
CDRNDataGridPendingWrite::GetSentResult( void ) const
{GUCEF_TRACE;

    return m_sent;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridPendingWrite::SetSentResult( const CORE::CDataTableWriteResult& sent )
{GUCEF_TRACE;

    m_sent = sent;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridPendingWrite::Start( UInt64 nowInMicroseconds )
{GUCEF_TRACE;

    m_started = true;
    m_deadline = 0 == m_timeoutMs ? 0 : nowInMicroseconds + (UInt64) m_timeoutMs * 1000;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridPendingWrite::HasStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridPendingWrite::AddAnswer( UInt32 proposalId, bool accepted, UInt32 voteOutcome, const CORE::CString& reason )
{GUCEF_TRACE;

    CAnswer answer;
    answer.proposalId = proposalId;
    answer.accepted = accepted;
    answer.voteOutcome = voteOutcome;
    answer.reason = reason;
    m_answers.push_back( answer );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridPendingWrite::IsComplete( void ) const
{GUCEF_TRACE;

    return m_started && m_answers.size() >= m_proposedRows.size();
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridPendingWrite::IsOverdue( UInt64 nowInMicroseconds ) const
{GUCEF_TRACE;

    return m_started && 0 != m_deadline && nowInMicroseconds >= m_deadline;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridPendingWrite::BuildResult( CORE::CDataTableWriteResult& result ) const
{GUCEF_TRACE;

    result = m_sent;

    // proposals are numbered in the order they were sent, so the answers by number are the answers by row
    TAnswerVector answers = m_answers;
    std::sort( answers.begin(), answers.end(), AnswerComesBefore );
    for ( UInt32 i=0; i<(UInt32) m_proposedRows.size() && i<(UInt32) answers.size(); ++i )
    {
        const UInt32 row = m_proposedRows[ i ];
        if ( row >= (UInt32) result.partitions.size() )
            continue;
        CORE::CDataTableWriteResult::SPartitionResult& partition = result.partitions[ row ];
        if ( answers[ i ].accepted )
        {
            partition.status = CORE::DATATABLE_STATUS_OK;
            partition.rowsWritten = m_changes[ row ].remove ? 0 : 1;
            partition.rowsDeleted = m_changes[ row ].remove ? 1 : 0;
            partition.message = "accepted";
        }
        else
        {
            partition.status = CORE::DATATABLE_STATUS_FAILED;
            partition.rowsWritten = 0;
            partition.rowsDeleted = 0;
            partition.message = answers[ i ].reason.IsNULLOrEmpty() ? CORE::CString( "refused" ) : answers[ i ].reason;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridPendingWrite::Complete( void )
{GUCEF_TRACE;

    CORE::CDataTableWriteResult result;
    BuildResult( result );
    return m_source.Complete( result.Clone(), true );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridPendingWrite::Fail( const CORE::CString& statusText )
{GUCEF_TRACE;

    return m_source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, statusText );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
