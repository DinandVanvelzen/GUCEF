/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CSchedulerChangeJournal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSchedulerChangeJournal::CSchedulerChangeJournal( UInt32 capacity )
    : m_ring()
    , m_capacity( capacity > 0 ? capacity : 1 )
    , m_start( 0 )
    , m_count( 0 )
    , m_floor( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSchedulerChangeJournal::Reset( UInt32 capacity, UInt64 floorRevision )
{GUCEF_TRACE;

    m_capacity = capacity > 0 ? capacity : 1;
    m_ring.clear();
    m_start = 0;
    m_count = 0;
    m_floor = floorRevision;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerChangeJournal::Add( UInt64 revision, Int32 kind, const CORE::CString& key )
{GUCEF_TRACE;

    SRecord record;
    record.revision = revision;
    record.kind = kind;
    record.key = key;
    if ( m_ring.size() < m_capacity )
    {
        m_ring.push_back( record );
        ++m_count;
        return;
    }
    // full: the oldest goes, the journal covers only what follows it
    SRecord& oldest = m_ring[ m_start ];
    if ( oldest.revision > m_floor )
        m_floor = oldest.revision;
    oldest = record;
    m_start = ( m_start + 1 ) % m_capacity;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerChangeJournal::Collect( UInt64 afterRevision, TRecordVector& records ) const
{GUCEF_TRACE;

    records.clear();
    if ( afterRevision < m_floor )
        return false;
    for ( UInt32 n=0; n<m_count; ++n )
    {
        const SRecord& record = m_ring[ ( m_start + n ) % m_ring.size() ];
        if ( record.revision > afterRevision )
            records.push_back( record );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedulerChangeJournal::GetFloorRevision( void ) const
{GUCEF_TRACE;

    return m_floor;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerChangeJournal::GetCount( void ) const
{GUCEF_TRACE;

    return m_count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
