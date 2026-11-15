/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CJobRunHistory.h"

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

CJobRunHistory::CJobRunHistory( UInt32 maximum )
    : m_ring()
    , m_next( 0 )
    , m_count( 0 )
    , m_maximum( maximum )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CJobRunHistory::SetMaximum( UInt32 maximum )
{GUCEF_TRACE;

    if ( maximum == m_maximum )
        return;

    // keep the newest runs, in order oldest to newest, then rebuild the ring with them
    TJobRunRecordVector newestFirst;
    GetNewestFirst( newestFirst, 0, maximum );
    m_ring.clear();
    m_next = 0;
    m_count = 0;
    m_maximum = maximum;
    if ( 0 != maximum )
        m_ring.reserve( maximum );
    for ( TJobRunRecordVector::reverse_iterator i = newestFirst.rbegin(); i != newestFirst.rend(); ++i )
        Add( (*i) );
}

/*-------------------------------------------------------------------------*/

UInt32
CJobRunHistory::GetMaximum( void ) const
{GUCEF_TRACE;

    return m_maximum;
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistory::Add( const SJobRunRecord& run )
{GUCEF_TRACE;

    if ( 0 == m_maximum || m_ring.size() < m_maximum )
    {
        m_ring.push_back( run );
        m_count = (UInt32) m_ring.size();
        m_next = 0 == m_maximum ? 0 : ( m_count % m_maximum );
        return;
    }

    // full: the slot of the oldest run is reused
    m_ring[ m_next ] = run;
    m_next = ( m_next + 1 ) % m_maximum;
}

/*-------------------------------------------------------------------------*/

const SJobRunRecord&
CJobRunHistory::GetByAge( UInt32 newestFirstIndex ) const
{GUCEF_TRACE;

    // while not full (or without a maximum) the newest is the last one pushed; once full the newest is before m_next
    UInt32 size = (UInt32) m_ring.size();
    if ( 0 == m_maximum || size < m_maximum )
        return m_ring[ size - 1 - newestFirstIndex ];
    UInt32 newest = ( m_next + size - 1 ) % size;
    return m_ring[ ( newest + size - newestFirstIndex ) % size ];
}

/*-------------------------------------------------------------------------*/

bool
CJobRunHistory::Find( UInt64 runId, SJobRunRecord& run ) const
{GUCEF_TRACE;

    for ( TJobRunRecordVector::const_iterator i = m_ring.begin(); i != m_ring.end(); ++i )
    {
        if ( (*i).runId == runId )
        {
            run = (*i);
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobRunHistory::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_ring.size();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistory::GetNewestFirst( TJobRunRecordVector& runs, UInt64 changedAfterRevision, UInt32 maxRuns ) const
{GUCEF_TRACE;

    UInt32 size = (UInt32) m_ring.size();
    UInt32 added = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( 0 != maxRuns && added >= maxRuns )
            break;
        const SJobRunRecord& run = GetByAge( i );
        if ( run.revision > changedAfterRevision )
        {
            runs.push_back( run );
            ++added;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistory::Clear( void )
{GUCEF_TRACE;

    m_ring.clear();
    m_next = 0;
    m_count = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
