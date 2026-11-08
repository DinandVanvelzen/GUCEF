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

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CIPulseGeneratorDriver
{
    public:

    CIPulseGeneratorDriver( void );

    CIPulseGeneratorDriver( const CIPulseGeneratorDriver& src );

    virtual ~CIPulseGeneratorDriver();

    CIPulseGeneratorDriver& operator=( const CIPulseGeneratorDriver& src );

    virtual void RequestImmediatePulse( CPulseGenerator& pulseGenerator ) = 0;

    virtual void RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                        const Int32 requestedPulsesPerImmediatePulseRequest ) = 0;

    virtual void RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                        const UInt32 pulseDeltaInMilliSecs ) = 0;

    virtual void RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs ) = 0;

    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator ) = 0;

    /**
     *  Takes an immediate pulse request made through CPulseGenerator::RequestImmediatePulseThroughDriver(): from any
     *  thread, with no lock of the pulse generator held. It never pulses on the calling thread: the request is kept
     *  for the loop of the driver, and the driver ends the wait of that loop between its pulses where it can.
     *  The default passes the request to RequestImmediatePulse(), which suits drivers that never pulse inside that call
     *  (a task delegator, the gucefDRN node task, a driver that only counts requests). A driver that pulses inside
     *  RequestImmediatePulse() in some state (the busy wait and Emscripten drivers outside their loops) overrides this.
     *  CPulseGenerator::SetPulseGeneratorDriver() waits for calls in progress before it lets the driver go: the call must
     *  be short and must not wait for anything the code that changes the driver may hold.
     */
    virtual void QueueImmediatePulse( CPulseGenerator& pulseGenerator );
    
    virtual UInt32 GetPulseDriverThreadId( void ) const = 0;

    protected:

    /**
     *  Simply proxy to the generator intended to force the implementation of a proper
     *  pulse generator driver
     */
    void SendDriverPulse( CPulseGenerator& pulseGenerator );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-11-2004 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
