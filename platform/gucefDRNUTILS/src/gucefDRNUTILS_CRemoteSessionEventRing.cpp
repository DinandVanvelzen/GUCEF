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

#include "gucefDRNUTILS_CRemoteSessionEventRing.h"

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

CRemoteSessionEventRing::CRemoteSessionEventRing( void )
    : m_events()
    , m_head( 0 )
    , m_maxEvents( 4096 )
    , m_maxBytes( 2 * 1024 * 1024 )
    , m_bytes( 0 )
    , m_nextSeq( 1 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionEventRing::SetLimits( UInt32 maxEvents, UInt64 maxBytes )
{GUCEF_TRACE;

    m_maxEvents = maxEvents;
    m_maxBytes = maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteSessionEventRing::Append( COMCORE::CRpcMessage& event, UInt64 nowUtcMs )
{GUCEF_TRACE;

    const UInt64 seq = m_nextSeq++;
    event.SetUInt64( EventSeqField, seq );
    event.SetUInt64( AtUtcMsField, nowUtcMs );

    SEvent entry;
    entry.eventSeq = seq;
    entry.message = event;
    entry.bytes = event.GetEncodedSize();
    m_events.push_back( entry );
    m_bytes += entry.bytes;

    // the oldest go first; the newest event is always kept, also when it alone is beyond the byte limit
    while ( GetRetainedCount() > 1 &&
            ( ( 0 != m_maxEvents && GetRetainedCount() > m_maxEvents ) || ( 0 != m_maxBytes && m_bytes > m_maxBytes ) ) )
    {
        m_bytes -= m_events[ m_head ].bytes;
        m_events[ m_head ].message.Reset();
        ++m_head;
    }
    if ( m_head > 64 && m_head * 2 > (UInt32) m_events.size() )
    {
        m_events.erase( m_events.begin(), m_events.begin() + m_head );
        m_head = 0;
    }
    return seq;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteSessionEventRing::GetNextSeq( void ) const
{GUCEF_TRACE;

    return m_nextSeq;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteSessionEventRing::GetRetainedFrom( void ) const
{GUCEF_TRACE;

    if ( 0 == GetRetainedCount() )
        return m_nextSeq;
    return m_events[ m_head ].eventSeq;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteSessionEventRing::GetRetainedCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_events.size() - m_head;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteSessionEventRing::GetRetainedBytes( void ) const
{GUCEF_TRACE;

    return m_bytes;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteSessionEventRing::Covers( UInt64 fromSeq ) const
{GUCEF_TRACE;

    if ( 0 == fromSeq )
        return true;    // "from now"
    // from the next sequence on nothing was missed; from before the oldest kept something was
    if ( fromSeq >= m_nextSeq )
        return true;
    return fromSeq >= GetRetainedFrom();
}

/*-------------------------------------------------------------------------*/

const CRemoteSessionEventRing::SEvent*
CRemoteSessionEventRing::Find( UInt64 eventSeq ) const
{GUCEF_TRACE;

    if ( 0 == GetRetainedCount() || eventSeq < GetRetainedFrom() || eventSeq >= m_nextSeq )
        return GUCEF_NULL;
    // sequence numbers are consecutive within the ring
    UInt64 index = m_head + ( eventSeq - GetRetainedFrom() );
    if ( index >= m_events.size() )
        return GUCEF_NULL;
    return &m_events[ (UInt32) index ];
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionEventRing::Clear( void )
{GUCEF_TRACE;

    m_events.clear();
    m_head = 0;
    m_bytes = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
