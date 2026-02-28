/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
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
    , m_desiredPulseDelta( 10 )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_pulseDriverThreadId( MT::GetCurrentTaskID() )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CBusyWaitPulseGeneratorDriver::CBusyWaitPulseGeneratorDriver( const CBusyWaitPulseGeneratorDriver& src )
    : CIPulseGeneratorDriver( src ) 
    , m_loop( false )               
    , m_desiredPulseDelta( 10 )     
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_pulseDriverThreadId( MT::GetCurrentTaskID() )
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
        m_immediatePulseTickets = m_immediatePulseTicketMax;
    }
    else
        SendDriverPulse( pulseGenerator );
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

    m_pulseDriverThreadId = MT::GetCurrentTaskID();

    m_loop = true;
    while ( m_loop )
    {
        try
        {
            // For immediate pulses we use a tight loop with no CPU yielding at all
            // This is intended for when the application is stressed and CPU bottlenecking on its workload
            if ( m_immediatePulseTickets > 0 )
            {
                --m_immediatePulseTickets;
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

    m_desiredPulseDelta = pulseDeltaInMilliSecs;                                      
}

/*--------------------------------------------------------------------------*/

void
CBusyWaitPulseGeneratorDriver::RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                                     const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    m_desiredPulseDelta = pulseDeltaInMilliSecs;
}

/*--------------------------------------------------------------------------*/
    
void
CBusyWaitPulseGeneratorDriver::RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    m_loop = false;
}

/*--------------------------------------------------------------------------*/

UInt32
CBusyWaitPulseGeneratorDriver::GetPulseDriverThreadId( void ) const 
{GUCEF_TRACE;

    return m_pulseDriverThreadId;
}   

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
