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

#ifndef GUCEF_CORE_CEMSCRIPTENPULSEGENERATORDRIVER_H
#define GUCEF_CORE_CEMSCRIPTENPULSEGENERATORDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

/**
 *  Pulse generator driver for applications running in a web browser.
 *
 *  A browser does not allow an application to block its main thread: the tab freezes and events like
 *  input, rendering and network are not delivered. A driver that sleeps between pulses, like the busy wait
 *  driver, can therefore not be used. This driver instead lets the browser call us: it hands the main
 *  loop over to emscripten_set_main_loop_arg() and sends one pulse per call.
 *
 *  Two timing modes are available:
 *   - Animation frame: one pulse per display refresh. This is the right choice for anything that renders.
 *     If a pulse interval is requested that is longer than a frame, frames are skipped to honor it.
 *   - Timeout: pulses at the requested interval independent of the display, for applications that do not render.
 */
class GUCEF_CORE_PUBLIC_CPP CEmscriptenPulseGeneratorDriver : public CIPulseGeneratorDriver
{
    public:

    enum ETimingMode
    {
        TIMING_ANIMATION_FRAME = 0 ,
        TIMING_TIMEOUT
    };
    typedef enum ETimingMode TTimingMode;

    CEmscriptenPulseGeneratorDriver( void );

    virtual ~CEmscriptenPulseGeneratorDriver() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Hands the main loop to the browser, which starts sending pulses to the given generator.
     *
     *  @param simulateInfiniteLoop if true this call never returns, the C++ stack is abandoned (without
     *         running destructors) and the browser keeps calling us. This is the safest choice for a main().
     *         If false the call returns right away and the caller has to keep all state alive.
     */
    void Run( CPulseGenerator& pulseGenerator    ,
              bool simulateInfiniteLoop = true   );

    /** Stops the main loop. Pulses are no longer sent. */
    void Stop( void );

    bool IsRunning( void ) const;

    void SetTimingMode( const TTimingMode timingMode );

    TTimingMode GetTimingMode( void ) const;

    virtual void RequestImmediatePulse( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** Keeps the request for the browser main loop, also before it runs; never pulses here */
    virtual void QueueImmediatePulse( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                        const Int32 requestedPulsesPerImmediatePulseRequest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                        const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TThreadId GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TSystemThreadId GetPulseDriverSystemThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEmscriptenPulseGeneratorDriver( const CEmscriptenPulseGeneratorDriver& src );
    CEmscriptenPulseGeneratorDriver& operator=( const CEmscriptenPulseGeneratorDriver& src );

    static void MainLoopCallback( void* driver );

    void OnMainLoopCycle( void );

    void ApplyTiming( void );

    private:

    CPulseGenerator* m_pulseGenerator;
    bool m_running;
    TTimingMode m_timingMode;
    UInt32 m_desiredPulseDelta;
    Int32 m_immediatePulseTickets;
    Int32 m_immediatePulseTicketMax;
    Float64 m_lastPulseTime;
    TThreadId m_pulseDriverThreadId;
    GUCEF_SYSTEM_THREAD_ID_MEMBER( m_pulseDriverSystemThreadId )   /**< display only, written with m_pulseDriverThreadId */
};

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEMSCRIPTENPULSEGENERATORDRIVER_H ? */
