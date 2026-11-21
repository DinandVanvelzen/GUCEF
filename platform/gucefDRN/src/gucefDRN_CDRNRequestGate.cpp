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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#include "gucefDRN_CDRNRequestGate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A caller in the queue: it waits on its own semaphore, which the giver of the budget posts when its turn came */
class CDRNRequestGate::CTicket
{
    public:

    MT::CSemaphore turn;
    bool granted;

    CTicket( void ) : turn( 0 ), granted( false ) {}
};

/*-------------------------------------------------------------------------*/

/** The wait of a pump thread: its turn came */
class CDRNRequestGate::CTurnCondition : public CIDRNWaitCondition
{
    public:

    CDRNRequestGate& gate;
    CDRNRequestGate::CTicket& ticket;

    CTurnCondition( CDRNRequestGate& g, CDRNRequestGate::CTicket& t ) : CIDRNWaitCondition(), gate( g ), ticket( t ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        MT::CScopeMutex lock( gate.m_lock );
        return ticket.granted;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRequestGate::CDRNRequestGate( UInt32 limit )
    : m_lock()
    , m_limit( limit )
    , m_inFlight( 0 )
    , m_peakInFlight( 0 )
    , m_queue()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRequestGate::~CDRNRequestGate()
{GUCEF_TRACE;

    // a gate is not destroyed while callers wait on it, as a semaphore is not
    for ( UInt32 i=0; i<m_queue.size(); ++i )
        GUCEF_DELETE m_queue[ i ];
    m_queue.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNRequestGate::SetLimit( UInt32 limit )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_limit = limit;
    GrantWaiting();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRequestGate::GetLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_limit;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRequestGate::Take( void )
{GUCEF_TRACE;

    // caller holds the lock
    if ( 0 != m_limit && m_inFlight >= m_limit )
        return false;
    ++m_inFlight;
    if ( m_inFlight > m_peakInFlight )
        m_peakInFlight = m_inFlight;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRequestGate::GrantWaiting( void )
{GUCEF_TRACE;

    // caller holds the lock: the head of the queue goes first, and nobody passes it
    while ( !m_queue.empty() && Take() )
    {
        CTicket* ticket = m_queue.front();
        m_queue.erase( m_queue.begin() );
        ticket->granted = true;
        ticket->turn.Post();
    }
}

/*-------------------------------------------------------------------------*/

CDRNRequestGate::CTicket*
CDRNRequestGate::Enter( void )
{GUCEF_TRACE;

    // caller holds the lock; NULL: the budget was taken at once
    if ( m_queue.empty() && Take() )
        return GUCEF_NULL;
    CTicket* ticket = GUCEF_NEW CTicket();
    m_queue.push_back( ticket );
    return ticket;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRequestGate::Leave( CTicket* ticket )
{GUCEF_TRACE;

    // the end of a wait, with the lock taken here: whether the budget was granted before the caller gave up
    MT::CScopeMutex lock( m_lock );
    bool granted = ticket->granted;
    if ( !granted )
    {
        for ( UInt32 i=0; i<m_queue.size(); ++i )
        {
            if ( m_queue[ i ] == ticket )
            {
                m_queue.erase( m_queue.begin() + i );
                break;
            }
        }
    }
    GUCEF_DELETE ticket;
    return granted;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRequestGate::Acquire( UInt32 timeoutInMs )
{GUCEF_TRACE;

    CTicket* ticket = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        ticket = Enter();
    }
    if ( GUCEF_NULL == ticket )
        return true;
    ticket->turn.WaitOrTimeout( timeoutInMs );
    return Leave( ticket );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRequestGate::Acquire( UInt32 timeoutInMs, CDRNNode& node )
{GUCEF_TRACE;

    CTicket* ticket = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        ticket = Enter();
    }
    if ( GUCEF_NULL == ticket )
        return true;
    if ( node.IsPumpThread() )
    {
        CTurnCondition turn( *this, *ticket );
        node.PumpUntil( turn, timeoutInMs );
    }
    else
    {
        ticket->turn.WaitOrTimeout( timeoutInMs );
    }
    return Leave( ticket );
}

/*-------------------------------------------------------------------------*/

void
CDRNRequestGate::Release( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_inFlight > 0 )
        --m_inFlight;
    GrantWaiting();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRequestGate::GetInFlight( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_inFlight;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRequestGate::GetWaiting( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_queue.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRequestGate::GetPeakInFlight( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_peakInFlight;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
