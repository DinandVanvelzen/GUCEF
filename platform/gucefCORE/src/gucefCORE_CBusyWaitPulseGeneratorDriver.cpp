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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CBusyWaitPulseGeneratorDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { 
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CBusyWaitPulseGeneratorDriver::CBusyWaitPulseGeneratorDriver( void )
    : CIPulseGeneratorDriver()
    , m_loop( false )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_pulseDriverThreadId( MT::GetCurrentThreadID() )
    GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( m_pulseDriverSystemThreadId, MT::GetCurrentThreadSystemID() )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CBusyWaitPulseGeneratorDriver::CBusyWaitPulseGeneratorDriver( const CBusyWaitPulseGeneratorDriver& src )
    : CIPulseGeneratorDriver( src )
    , m_loop( false )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_pulseDriverThreadId( MT::GetCurrentThreadID() )
    GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( m_pulseDriverSystemThreadId, MT::GetCurrentThreadSystemID() )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CBusyWaitPulseGeneratorDriver::~CBusyWaitPulseGeneratorDriver()
{GUCEF_TRACE;

    m_loop = false;
}

/*--------------------------------------------------------------------------*/

void
CBusyWaitPulseGeneratorDriver::RequestImmediatePulse( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( m_loop )
    {
        m_immediatePulseTickets.Store( m_immediatePulseTicketMax.Load() );
    }
    else
        SendDriverPulse( pulseGenerator );
}

/*--------------------------------------------------------------------------*/

void
CBusyWaitPulseGeneratorDriver::QueueImmediatePulse( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    // Whether or not the loop runs: never a pulse on the calling thread, which may be any thread. A loop that runs
    // takes the ticket at its next pulse, a loop that starts later at its first
    m_immediatePulseTickets.Store( m_immediatePulseTicketMax.Load() );
}

/*--------------------------------------------------------------------------*/

void 
CBusyWaitPulseGeneratorDriver::RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                                      const Int32 requestedPulsesPerImmediatePulseRequest )
{GUCEF_TRACE;

    if ( requestedPulsesPerImmediatePulseRequest > 1 )
        m_immediatePulseTicketMax = requestedPulsesPerImmediatePulseRequest;
    else
        m_immediatePulseTicketMax = 1;
}

/*--------------------------------------------------------------------------*/

void
CBusyWaitPulseGeneratorDriver::Run( CPulseGenerator& pulseGenerator            ,
                                    UInt32 forcedMinimalCycleDeltaInMilliSecs  ,
                                    UInt32 desiredMaximumCycleDeltaInMilliSecs )
{GUCEF_TRACE;

    MT::SThreadIds driverIds;
    MT::GetCurrentThreadIds( &driverIds );
    m_pulseDriverThreadId = driverIds.threadId;
    GUCEF_SYSTEM_THREAD_ID_CODE( m_pulseDriverSystemThreadId = driverIds.systemThreadId; )

    m_loop = true;
    while ( m_loop )
    {
        try
        {
            // For immediate pulses we use a tight loop with no CPU yielding at all
            // This is intended for when the application is stressed and CPU bottlenecking on its workload.
            // A ticket is taken only when it is still there: other threads hand out new ones meanwhile
            Int32 tickets = m_immediatePulseTickets.Load();
            if ( tickets > 0 )
            {
                if ( m_immediatePulseTickets.CompareExchangeStrong( tickets, tickets - 1 ) )
                    SendDriverPulse( pulseGenerator );
                continue;
            }

            pulseGenerator.WaitTillNextPulseWindow( forcedMinimalCycleDeltaInMilliSecs, desiredMaximumCycleDeltaInMilliSecs );
            SendDriverPulse( pulseGenerator );
        }
        catch ( const timeout_exception& )
        {
            // just go for another round if we hit a timeout
        }
    }
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BusyWaitPulseGeneratorDriver: Exited run loop" );
}

/*--------------------------------------------------------------------------*/
    
void
CBusyWaitPulseGeneratorDriver::RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                                      const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    // Nothing to keep: the loop waits for the next pulse window of the pulse generator, which has the interval
}

/*--------------------------------------------------------------------------*/

void
CBusyWaitPulseGeneratorDriver::RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                                     const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    // Nothing to keep: the loop waits for the next pulse window of the pulse generator, which has the interval
}

/*--------------------------------------------------------------------------*/
    
void
CBusyWaitPulseGeneratorDriver::RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    m_loop = false;
}

/*--------------------------------------------------------------------------*/

TThreadId
CBusyWaitPulseGeneratorDriver::GetPulseDriverThreadId( void ) const 
{GUCEF_TRACE;

    return m_pulseDriverThreadId;
}   
/*--------------------------------------------------------------------------*/

TSystemThreadId
CBusyWaitPulseGeneratorDriver::GetPulseDriverSystemThreadId( void ) const 
{GUCEF_TRACE;

    return GUCEF_SYSTEM_THREAD_ID_OR_THREAD_ID( m_pulseDriverSystemThreadId.Load(), m_pulseDriverThreadId.Load() );
}   

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
