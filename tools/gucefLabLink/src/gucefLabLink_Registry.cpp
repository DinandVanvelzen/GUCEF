/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#include "gucefLabLink_Registry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What is kept when nobody collects: older events and forgotten ids are dropped first */
static const size_t MaxPendingEvents = 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNodeRegistry::CNodeRegistry( const SRegistryLimits& limits )
    : m_limits( limits )
    , m_entries()
    , m_events()
    , m_forgotten()
    , m_startInMonoMs( 0 )
    , m_started( false )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const SRegistryLimits&
CNodeRegistry::GetLimits( void ) const
{GUCEF_TRACE;

    return m_limits;
}

/*-------------------------------------------------------------------------*/

UInt32
CNodeRegistry::CountVerified( bool verified ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
        if ( (*i).second.verified == verified )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::AddEvent( UInt64 unixSecs, const CORE::CString& machineId, const char* kind, const CORE::CString& detail )
{GUCEF_TRACE;

    if ( m_events.size() >= MaxPendingEvents )
        m_events.erase( m_events.begin() );
    SNodeEvent event;
    event.unixSecs = unixSecs;
    event.machineId = machineId;
    event.kind = kind;
    event.detail = detail;
    m_events.push_back( event );
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::Load( const vector< SNodeEntry >& entries, UInt64 monoNowInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_started )
    {
        m_started = true;
        m_startInMonoMs = monoNowInMs;
    }
    for ( size_t i=0; i<entries.size(); ++i )
    {
        const SNodeEntry& source = entries[ i ];
        if ( source.report.machineId.IsNULLOrEmpty() || m_entries.find( source.report.machineId ) != m_entries.end() )
            continue;
        if ( CountVerified( source.verified ) >= ( source.verified ? m_limits.maxVerifiedNodes : m_limits.maxUnverifiedNodes ) )
            continue;
        SNodeEntry entry = source;
        entry.liveness = NODELIVENESS_UNKNOWN;
        entry.lastSeenInMonoMs = 0;
        entry.dirty = false;
        entry.duplicateInstanceRefusals = 0;
        m_entries[ entry.report.machineId ] = entry;
    }
}

/*-------------------------------------------------------------------------*/

EReportResult
CNodeRegistry::OnReport( const SNodeReport& report, bool verified, const CORE::CString& address, UInt64 monoNowInMs, UInt64 unixNowInSecs )
{GUCEF_TRACE;

    if ( report.machineId.IsNULLOrEmpty() )
        return REPORT_INVALID;

    MT::CScopeMutex lock( m_lock );

    if ( !m_started )
    {
        m_started = true;
        m_startInMonoMs = monoNowInMs;
    }

    TEntryMap::iterator found = m_entries.find( report.machineId );
    if ( found == m_entries.end() )
    {
        if ( CountVerified( verified ) >= ( verified ? m_limits.maxVerifiedNodes : m_limits.maxUnverifiedNodes ) )
            return REPORT_TABLE_FULL;

        SNodeEntry entry;
        entry.report = report;
        entry.verified = verified;
        entry.address = address;
        entry.liveness = NODELIVENESS_ONLINE;
        entry.firstSeenInUnixSecs = unixNowInSecs;
        entry.lastSeenInUnixSecs = unixNowInSecs;
        entry.lastSeenInMonoMs = 0 == monoNowInMs ? 1 : monoNowInMs;
        entry.dirty = true;
        m_entries[ report.machineId ] = entry;
        AddEvent( unixNowInSecs, report.machineId, "online", "first report" );
        return REPORT_ACCEPTED;
    }

    SNodeEntry& entry = (*found).second;

    // an unverified reporter holds a token it made itself: it cannot speak for a node that was approved
    if ( entry.verified && !verified )
        return REPORT_UNVERIFIED_CLAIM;

    const bool upgrading = !entry.verified && verified;
    if ( upgrading && CountVerified( true ) >= m_limits.maxVerifiedNodes )
        return REPORT_TABLE_FULL;

    // another run of the agent under this machine id. A restart is the newer run taking over at once. An older run
    // that reports while the newer one is heard from is a second instance (a cloned disk): refused and counted, so
    // the two never mix, and the answer does not flip between them
    if ( !upgrading && !entry.report.instanceId.IsNULLOrEmpty() && !report.instanceId.IsNULLOrEmpty() && entry.report.instanceId != report.instanceId )
    {
        const bool heardRecently = 0 != entry.lastSeenInMonoMs && monoNowInMs >= entry.lastSeenInMonoMs &&
                                   ( monoNowInMs - entry.lastSeenInMonoMs ) <= (UInt64) m_limits.offlineAfterInSecs * 1000;
        const bool newerRun = 0 != report.startedInUnixSecs && report.startedInUnixSecs > entry.report.startedInUnixSecs;
        if ( heardRecently && !newerRun )
        {
            ++entry.duplicateInstanceRefusals;
            return REPORT_DUPLICATE_INSTANCE;
        }
        AddEvent( unixNowInSecs, report.machineId, "instance", newerRun ? "a newer run of the agent took over" : "another run of the agent took over" );
        entry.dirty = true;
    }

    if ( upgrading )
    {
        entry.verified = true;
        AddEvent( unixNowInSecs, report.machineId, "verified", "reported with the token of its approval" );
        entry.dirty = true;
    }
    if ( entry.report.nodeState != report.nodeState )
    {
        AddEvent( unixNowInSecs, report.machineId, "state", entry.report.nodeState + " -> " + report.nodeState );
        entry.dirty = true;
    }
    if ( NODELIVENESS_ONLINE != entry.liveness )
    {
        AddEvent( unixNowInSecs, report.machineId, "online", NODELIVENESS_OFFLINE == entry.liveness ? "back" : "first report since the hub started" );
        entry.dirty = true;
    }

    entry.report = report;
    entry.address = address;
    entry.liveness = NODELIVENESS_ONLINE;
    entry.lastSeenInUnixSecs = unixNowInSecs;
    entry.lastSeenInMonoMs = 0 == monoNowInMs ? 1 : monoNowInMs;
    return REPORT_ACCEPTED;
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::Evaluate( UInt64 monoNowInMs, UInt64 unixNowInSecs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_started )
    {
        m_started = true;
        m_startInMonoMs = monoNowInMs;
    }

    const UInt64 offlineAfterInMs = (UInt64) m_limits.offlineAfterInSecs * 1000;
    TEntryMap::iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        SNodeEntry& entry = (*i).second;

        // a node not heard from for very long is forgotten; the wall clock going backwards never does
        const UInt64 age = unixNowInSecs > entry.lastSeenInUnixSecs ? unixNowInSecs - entry.lastSeenInUnixSecs : 0;
        if ( age > ( entry.verified ? m_limits.forgetVerifiedAfterInSecs : m_limits.forgetUnverifiedAfterInSecs ) )
        {
            if ( m_forgotten.size() >= MaxPendingEvents )
                m_forgotten.erase( m_forgotten.begin() );
            m_forgotten.push_back( (*i).first );
            m_entries.erase( i++ );
            continue;
        }

        if ( NODELIVENESS_ONLINE == entry.liveness )
        {
            if ( monoNowInMs >= entry.lastSeenInMonoMs && ( monoNowInMs - entry.lastSeenInMonoMs ) > offlineAfterInMs )
            {
                entry.liveness = NODELIVENESS_OFFLINE;
                entry.dirty = true;
                AddEvent( unixNowInSecs, (*i).first, "offline", "no report" );
            }
        }
        else
        if ( NODELIVENESS_UNKNOWN == entry.liveness )
        {
            // it had one window to report after the hub started; what is stored already says when it was last seen
            if ( monoNowInMs >= m_startInMonoMs && ( monoNowInMs - m_startInMonoMs ) > offlineAfterInMs )
                entry.liveness = NODELIVENESS_OFFLINE;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CNodeRegistry::Find( const CORE::CString& machineId, SNodeEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( machineId );
    if ( i == m_entries.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::GetAll( vector< SNodeEntry >& entries ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    entries.clear();
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
        entries.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

UInt32
CNodeRegistry::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::TakeEvents( vector< SNodeEvent >& events )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    events = m_events;
    m_events.clear();
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::TakeForgotten( CORE::CString::StringVector& machineIds )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    machineIds = m_forgotten;
    m_forgotten.clear();
}

/*-------------------------------------------------------------------------*/

void
CNodeRegistry::CollectForPersist( UInt64 unixNowInSecs, vector< SNodeEntry >& entries )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    entries.clear();
    for ( TEntryMap::iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        SNodeEntry& entry = (*i).second;
        const bool flushDue = NODELIVENESS_ONLINE == entry.liveness && unixNowInSecs >= entry.lastPersistedInUnixSecs &&
                              ( unixNowInSecs - entry.lastPersistedInUnixSecs ) >= m_limits.flushIntervalInSecs;
        if ( !entry.dirty && !flushDue )
            continue;
        entry.dirty = false;
        entry.lastPersistedInUnixSecs = unixNowInSecs;
        entries.push_back( entry );
    }
}

void
CNodeRegistry::MarkAllDirty( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TEntryMap::iterator i = m_entries.begin(); i != m_entries.end(); ++i )
        (*i).second.dirty = true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
