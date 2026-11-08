/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcDurableRequestTable.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcDurableRequestTable::SEntry::SEntry( void )
    : id()
    , ownerSubject()
    , state( RPC_REQUESTSTATE_QUEUED )
    , progressPermille( 1001 )
    , etaMs( 0 )
    , note()
    , hasResult( false )
    , resultStatus()
    , resultPayload()
    , cancelRequested( false )
    , expiresAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CRpcDurableRequestTable::NowMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution < 1000 )
        return ticks;
    return ticks / ( resolution / 1000 );
}

/*-------------------------------------------------------------------------*/

CRpcDurableRequestTable::CRpcDurableRequestTable( UInt32 retentionMs, UInt32 maxPerSubject )
    : CORE::CTSharedObjCreator< CRpcDurableRequestTable, MT::CMutex >( this )
    , m_lock()
    , m_entries()
    , m_retentionMs( retentionMs )
    , m_maxPerSubject( maxPerSubject )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcDurableRequestTable::~CRpcDurableRequestTable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcDurableRequestTablePtr
CRpcDurableRequestTable::Create( UInt32 retentionMs, UInt32 maxPerSubject )
{GUCEF_TRACE;

    CRpcDurableRequestTable* table = GUCEF_NEW CRpcDurableRequestTable( retentionMs, maxPerSubject );
    return table->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

bool
CRpcDurableRequestTable::Add( const CRpcRequestId& id, const CORE::CString& ownerSubject )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 != m_maxPerSubject )
    {
        UInt32 live = 0;
        TEntryMap::const_iterator i = m_entries.begin();
        for ( ; i != m_entries.end(); ++i )
            if ( (*i).second.ownerSubject == ownerSubject && !(*i).second.hasResult )
                ++live;
        if ( live >= m_maxPerSubject )
            return false;
    }
    SEntry entry;
    entry.id = id;
    entry.ownerSubject = ownerSubject;
    entry.state = RPC_REQUESTSTATE_RUNNING;
    m_entries[ id ] = entry;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcDurableRequestTable::Get( const CRpcRequestId& id, SEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( id );
    if ( i == m_entries.end() )
        return false;
    if ( 0 != (*i).second.expiresAtMs && NowMs() > (*i).second.expiresAtMs )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcDurableRequestTable::SetProgress( const CRpcRequestId& id, TRpcRequestState state, UInt32 progressPermille, UInt32 etaMs, const CORE::CString& note )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( id );
    if ( i == m_entries.end() || (*i).second.hasResult )
        return false;
    (*i).second.state = state;
    (*i).second.progressPermille = progressPermille;
    (*i).second.etaMs = etaMs;
    (*i).second.note = note;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcDurableRequestTable::Complete( const CRpcRequestId& id, const CRpcStatus& status, const CORE::CDynamicBuffer* payload )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( id );
    if ( i == m_entries.end() || (*i).second.hasResult )
        return false;
    SEntry& entry = (*i).second;
    entry.hasResult = true;
    entry.resultStatus = status;
    entry.resultPayload.Clear( true );
    if ( GUCEF_NULL != payload && payload->GetDataSize() > 0 )
        entry.resultPayload.CopyFrom( payload->GetDataSize(), payload->GetConstBufferPtr() );
    if ( status.IsOk() )
        entry.state = RPC_REQUESTSTATE_DONE;
    else if ( RPC_STATUS_CANCELLED == status.code )
        entry.state = RPC_REQUESTSTATE_CANCELLED;
    else
        entry.state = RPC_REQUESTSTATE_FAILED;
    entry.progressPermille = entry.state == RPC_REQUESTSTATE_DONE ? 1000 : entry.progressPermille;
    entry.expiresAtMs = NowMs() + m_retentionMs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcDurableRequestTable::RequestCancel( const CRpcRequestId& id )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( id );
    if ( i == m_entries.end() )
        return false;
    (*i).second.cancelRequested = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcDurableRequestTable::IsCancelRequested( const CRpcRequestId& id ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( id );
    return i != m_entries.end() && (*i).second.cancelRequested;
}

/*-------------------------------------------------------------------------*/

void
CRpcDurableRequestTable::Expire( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = NowMs();
    TEntryMap::iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        if ( 0 != (*i).second.expiresAtMs && now > (*i).second.expiresAtMs )
            m_entries.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcDurableRequestTable::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
