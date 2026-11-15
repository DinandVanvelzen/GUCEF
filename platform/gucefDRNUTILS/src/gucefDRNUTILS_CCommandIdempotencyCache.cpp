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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefDRNUTILS_CCommandIdempotencyCache.h"

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

static CORE::CString
MakeKey( const CORE::CString& subject, const CORE::CString& commandId )
{GUCEF_TRACE;

    return subject + "\n" + commandId;
}

/*-------------------------------------------------------------------------*/

CCommandIdempotencyCache::CCommandIdempotencyCache( void )
    : m_lock()
    , m_ttlMs( 86400000 )
    , m_maxPerSubject( 256 )
    , m_maxTotal( 4096 )
    , m_nextSequence( 1 )
    , m_entries()
    , m_order()
    , m_subjectCounts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_ttlMs = ttlMs;
    m_maxPerSubject = maxPerSubject;
    m_maxTotal = maxTotal;
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::Remove( const CORE::CString& key )
{GUCEF_TRACE;

    TEntryMap::iterator i = m_entries.find( key );
    if ( i == m_entries.end() )
        return;
    m_order.erase( (*i).second.sequence );
    TSubjectCountMap::iterator s = m_subjectCounts.find( (*i).second.subject );
    if ( s != m_subjectCounts.end() )
    {
        if ( (*s).second <= 1 )
            m_subjectCounts.erase( s );
        else
            --(*s).second;
    }
    m_entries.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::Expire( UInt64 nowMs )
{GUCEF_TRACE;

    if ( 0 == m_ttlMs )
        return;
    // entries are stored in time order: the oldest are in front
    while ( !m_order.empty() )
    {
        CORE::CString key = (*m_order.begin()).second;
        TEntryMap::iterator i = m_entries.find( key );
        if ( i == m_entries.end() )
        {
            m_order.erase( m_order.begin() );
            continue;
        }
        if ( (*i).second.storedAtMs + m_ttlMs > nowMs )
            break;
        Remove( key );
    }
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::RemoveOldestOfSubject( const CORE::CString& subject )
{GUCEF_TRACE;

    TOrderMap::iterator o = m_order.begin();
    while ( o != m_order.end() )
    {
        TEntryMap::iterator i = m_entries.find( (*o).second );
        if ( i != m_entries.end() && (*i).second.subject == subject )
        {
            CORE::CString key = (*o).second;
            Remove( key );
            return;
        }
        ++o;
    }
}

/*-------------------------------------------------------------------------*/

bool
CCommandIdempotencyCache::Find( const CORE::CString& subject, const CORE::CString& commandId, UInt64 nowMs,
                                TDrnUtilsCommandStatus& status, CORE::SJobControlResult& result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Expire( nowMs );
    TEntryMap::iterator i = m_entries.find( MakeKey( subject, commandId ) );
    if ( i == m_entries.end() )
        return false;
    status = (*i).second.status;
    result = (*i).second.result;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::Store( const CORE::CString& subject, const CORE::CString& commandId, UInt64 nowMs,
                                 TDrnUtilsCommandStatus status, const CORE::SJobControlResult& result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Expire( nowMs );
    CORE::CString key = MakeKey( subject, commandId );
    Remove( key );
    if ( m_maxPerSubject > 0 )
    {
        TSubjectCountMap::iterator s = m_subjectCounts.find( subject );
        while ( s != m_subjectCounts.end() && (*s).second >= m_maxPerSubject )
        {
            RemoveOldestOfSubject( subject );
            s = m_subjectCounts.find( subject );
        }
    }
    if ( m_maxTotal > 0 )
    {
        while ( m_entries.size() >= m_maxTotal && !m_order.empty() )
        {
            CORE::CString oldest = (*m_order.begin()).second;
            Remove( oldest );
        }
    }
    SEntry& entry = m_entries[ key ];
    entry.storedAtMs = nowMs;
    entry.sequence = m_nextSequence++;
    entry.subject = subject;
    entry.status = status;
    entry.result = result;
    m_order[ entry.sequence ] = key;
    ++m_subjectCounts[ subject ];
}

/*-------------------------------------------------------------------------*/

UInt32
CCommandIdempotencyCache::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
    m_order.clear();
    m_subjectCounts.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
