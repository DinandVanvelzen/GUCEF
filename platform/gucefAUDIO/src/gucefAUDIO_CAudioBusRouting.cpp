/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOBUSROUTING_H
#include "gucefAUDIO_CAudioBusRouting.h"
#define GUCEF_AUDIO_CAUDIOBUSROUTING_H
#endif /* GUCEF_AUDIO_CAUDIOBUSROUTING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioBusRouting::CAudioBusRouting( UInt32 maxBuses )
    : m_maxBuses( maxBuses > 0 ? maxBuses : 1 )
    , m_buses()
    , m_parents()
    , m_sends()
{GUCEF_TRACE;

    m_buses.reserve( m_maxBuses );
    m_parents.reserve( m_maxBuses );
}

/*-------------------------------------------------------------------------*/

CAudioBusRouting::~CAudioBusRouting()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioBusRouting::Clear( void )
{GUCEF_TRACE;

    m_buses.clear();
    m_parents.clear();
    m_sends.clear();
}

/*-------------------------------------------------------------------------*/

Int32
CAudioBusRouting::IndexOf( const CAudioBus* bus ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == bus )
        return -1;
    for ( size_t i=0; i<m_buses.size(); ++i )
        if ( m_buses[ i ] == bus )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::AddBus( CAudioBus* bus, CAudioBus* parent )
{GUCEF_TRACE;

    if ( GUCEF_NULL == bus || m_buses.size() >= m_maxBuses || IndexOf( bus ) >= 0 )
        return false;

    Int32 parentIndex = -1;
    if ( m_buses.empty() )
    {
        if ( GUCEF_NULL != parent )
            return false;               // the root has no parent
    }
    else
    {
        parentIndex = IndexOf( parent );
        if ( parentIndex < 0 )
            return false;
    }

    m_buses.push_back( bus );
    m_parents.push_back( parentIndex );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::RemoveBus( const CAudioBus* bus )
{GUCEF_TRACE;

    const Int32 index = IndexOf( bus );
    if ( index <= 0 )
        return false;                   // unknown, or the root
    for ( size_t i=0; i<m_parents.size(); ++i )
        if ( m_parents[ i ] == index )
            return false;               // it has children
    for ( size_t e=0; e<m_sends.size(); ++e )
        if ( m_sends[ e ].from == (UInt32) index || m_sends[ e ].to == (UInt32) index )
            return false;

    m_buses.erase( m_buses.begin() + index );
    m_parents.erase( m_parents.begin() + index );
    for ( size_t i=0; i<m_parents.size(); ++i )
        if ( m_parents[ i ] > index )
            --m_parents[ i ];
    for ( size_t e=0; e<m_sends.size(); ++e )
    {
        if ( m_sends[ e ].from > (UInt32) index ) --m_sends[ e ].from;
        if ( m_sends[ e ].to > (UInt32) index ) --m_sends[ e ].to;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::HasBus( const CAudioBus* bus ) const
{GUCEF_TRACE;

    return IndexOf( bus ) >= 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioBusRouting::GetBusCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_buses.size();
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::Reaches( const CAudioBus* from, const CAudioBus* to ) const
{GUCEF_TRACE;

    const Int32 start = IndexOf( from );
    const Int32 goal = IndexOf( to );
    if ( start < 0 || goal < 0 )
        return false;

    // depth first over parent and send connections
    const size_t count = m_buses.size();
    GUCEF::vector< UInt8 > seen( count, 0 );
    GUCEF::vector< UInt32 > pending;
    pending.reserve( count );

    // the start itself is not "reached" unless a path comes back to it
    if ( m_parents[ start ] >= 0 )
        pending.push_back( (UInt32) m_parents[ start ] );
    for ( size_t e=0; e<m_sends.size(); ++e )
        if ( m_sends[ e ].from == (UInt32) start )
            pending.push_back( m_sends[ e ].to );

    while ( !pending.empty() )
    {
        const UInt32 node = pending.back();
        pending.pop_back();
        if ( node == (UInt32) goal )
            return true;
        if ( 0 != seen[ node ] )
            continue;
        seen[ node ] = 1;
        if ( m_parents[ node ] >= 0 )
            pending.push_back( (UInt32) m_parents[ node ] );
        for ( size_t e=0; e<m_sends.size(); ++e )
            if ( m_sends[ e ].from == node )
                pending.push_back( m_sends[ e ].to );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::CanAddSend( const CAudioBus* from, const CAudioBus* to ) const
{GUCEF_TRACE;

    if ( from == to || !HasBus( from ) || !HasBus( to ) )
        return false;

    // from -> to closes a cycle when 'to' already feeds 'from'
    return !Reaches( to, from );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::AddSend( CAudioBus* from, CAudioBus* to )
{GUCEF_TRACE;

    if ( !CanAddSend( from, to ) )
        return false;

    SEdge edge;
    edge.from = (UInt32) IndexOf( from );
    edge.to = (UInt32) IndexOf( to );
    m_sends.push_back( edge );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::RemoveSend( const CAudioBus* from, const CAudioBus* to )
{GUCEF_TRACE;

    const Int32 a = IndexOf( from );
    const Int32 b = IndexOf( to );
    if ( a < 0 || b < 0 )
        return false;
    for ( size_t e=0; e<m_sends.size(); ++e )
    {
        if ( m_sends[ e ].from == (UInt32) a && m_sends[ e ].to == (UInt32) b )
        {
            m_sends.erase( m_sends.begin() + e );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusRouting::ComputeOrder( CAudioBus** order, UInt32 capacity, UInt32& count ) const
{GUCEF_TRACE;

    count = 0;
    const size_t total = m_buses.size();
    if ( GUCEF_NULL == order || capacity < total )
        return false;

    // feeders still to be processed, per bus
    GUCEF::vector< UInt32 > waitingFor( total, 0 );
    for ( size_t i=0; i<total; ++i )
        if ( m_parents[ i ] >= 0 )
            ++waitingFor[ (size_t) m_parents[ i ] ];
    for ( size_t e=0; e<m_sends.size(); ++e )
        ++waitingFor[ m_sends[ e ].to ];

    GUCEF::vector< UInt8 > done( total, 0 );
    for ( size_t step=0; step<total; ++step )
    {
        // of the buses that have all their input, the one created last
        Int32 next = -1;
        for ( size_t i=total; i>0; --i )
        {
            if ( 0 == done[ i - 1 ] && 0 == waitingFor[ i - 1 ] )
            {
                next = (Int32) ( i - 1 );
                break;
            }
        }
        if ( next < 0 )
            return false;           // a cycle: cannot happen, connections that make one are refused

        done[ (size_t) next ] = 1;
        order[ count++ ] = m_buses[ (size_t) next ];
        if ( m_parents[ (size_t) next ] >= 0 )
            --waitingFor[ (size_t) m_parents[ (size_t) next ] ];
        for ( size_t e=0; e<m_sends.size(); ++e )
            if ( m_sends[ e ].from == (UInt32) next )
                --waitingFor[ m_sends[ e ].to ];
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
