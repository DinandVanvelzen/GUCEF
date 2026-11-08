/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CEmscriptenPulseGeneratorDriver.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

#include <emscripten.h>

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

CEmscriptenPulseGeneratorDriver::CEmscriptenPulseGeneratorDriver( void )
    : CIPulseGeneratorDriver()
    , m_pulseGenerator( GUCEF_NULL )
    , m_running( false )
    , m_timingMode( TIMING_ANIMATION_FRAME )
    , m_desiredPulseDelta( 10 )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_lastPulseTime( 0 )
    , m_pulseDriverThreadId( MT::GetCurrentTaskID() )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CEmscriptenPulseGeneratorDriver::~CEmscriptenPulseGeneratorDriver()
{GUCEF_TRACE;

    Stop();
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::MainLoopCallback( void* driver )
{
    static_cast< CEmscriptenPulseGeneratorDriver* >( driver )->OnMainLoopCycle();
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::ApplyTiming( void )
{GUCEF_TRACE;

    if ( !m_running )
        return;

    if ( TIMING_TIMEOUT == m_timingMode )
        emscripten_set_main_loop_timing( EM_TIMING_SETTIMEOUT, m_desiredPulseDelta > 0 ? (int) m_desiredPulseDelta : 1 );
    else
        emscripten_set_main_loop_timing( EM_TIMING_RAF, 1 );
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::OnMainLoopCycle( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pulseGenerator )
        return;

    // Immediate pulses take precedence and are not subject to the pulse interval
    if ( m_immediatePulseTickets > 0 )
    {
        while ( m_immediatePulseTickets > 0 )
        {
            --m_immediatePulseTickets;
            SendDriverPulse( *m_pulseGenerator );
        }
        m_lastPulseTime = emscripten_get_now();
        return;
    }

    // With animation frame timing we are called per display refresh. A longer interval than that is
    // honored by skipping frames, a shorter interval cannot be honored in this mode.
    const Float64 now = emscripten_get_now();
    if ( TIMING_ANIMATION_FRAME == m_timingMode && ( now - m_lastPulseTime ) < (Float64) m_desiredPulseDelta )
        return;

    m_lastPulseTime = now;
    SendDriverPulse( *m_pulseGenerator );
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::Run( CPulseGenerator& pulseGenerator ,
                                      bool simulateInfiniteLoop       )
{GUCEF_TRACE;

    m_pulseDriverThreadId = MT::GetCurrentTaskID();
    m_pulseGenerator = &pulseGenerator;
    m_lastPulseTime = emscripten_get_now();
    m_running = true;

    // Frame rate 0 lets the browser decide, which is requestAnimationFrame
    emscripten_set_main_loop_arg( &CEmscriptenPulseGeneratorDriver::MainLoopCallback, this, 0, simulateInfiniteLoop ? 1 : 0 );
    ApplyTiming();
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::Stop( void )
{GUCEF_TRACE;

    if ( m_running )
    {
        m_running = false;
        emscripten_cancel_main_loop();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EmscriptenPulseGeneratorDriver: Stopped main loop" );
    }
}

/*--------------------------------------------------------------------------*/

bool
CEmscriptenPulseGeneratorDriver::IsRunning( void ) const
{GUCEF_TRACE;

    return m_running;
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::SetTimingMode( const TTimingMode timingMode )
{GUCEF_TRACE;

    m_timingMode = timingMode;
    ApplyTiming();
}

/*--------------------------------------------------------------------------*/

CEmscriptenPulseGeneratorDriver::TTimingMode
CEmscriptenPulseGeneratorDriver::GetTimingMode( void ) const
{GUCEF_TRACE;

    return m_timingMode;
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::RequestImmediatePulse( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( m_running )
    {
        // The next main loop cycle takes care of it. We must never pulse from here as the request
        // can come from within a pulse and recursing is exactly what the pulse generator wants to avoid.
        m_immediatePulseTickets = m_immediatePulseTicketMax;
    }
    else
        SendDriverPulse( pulseGenerator );
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::QueueImmediatePulse( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    // Running or not: a request through the driver may come from any thread and never pulses there. The main loop
    // takes it at its next cycle, or at its first when it starts later
    m_immediatePulseTickets = m_immediatePulseTicketMax;
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                                        const Int32 requestedPulsesPerImmediatePulseRequest )
{GUCEF_TRACE;

    m_immediatePulseTicketMax = requestedPulsesPerImmediatePulseRequest > 1 ? requestedPulsesPerImmediatePulseRequest : 1;
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                                        const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    m_desiredPulseDelta = pulseDeltaInMilliSecs;
    ApplyTiming();
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                                       const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    m_desiredPulseDelta = pulseDeltaInMilliSecs;
    ApplyTiming();
}

/*--------------------------------------------------------------------------*/

void
CEmscriptenPulseGeneratorDriver::RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    Stop();
}

/*--------------------------------------------------------------------------*/

UInt32
CEmscriptenPulseGeneratorDriver::GetPulseDriverThreadId( void ) const
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

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ? */

/*--------------------------------------------------------------------------*/
