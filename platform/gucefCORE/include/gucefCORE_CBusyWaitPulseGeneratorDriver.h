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

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CBusyWaitPulseGeneratorDriver : public CIPulseGeneratorDriver
{
    public:

    CBusyWaitPulseGeneratorDriver( void );

    CBusyWaitPulseGeneratorDriver( const CBusyWaitPulseGeneratorDriver& src );

    virtual ~CBusyWaitPulseGeneratorDriver();

    CBusyWaitPulseGeneratorDriver& operator=( const CIPulseGeneratorDriver& src );

    virtual void RequestImmediatePulse( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** Keeps the request for the loop of Run(), which takes it at its next pulse (at once when it starts later); never pulses here */
    virtual void QueueImmediatePulse( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                        const Int32 requestedPulsesPerImmediatePulseRequest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPeriodicPulses( CPulseGenerator& pulseGenerator   ,
                                        const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulseInterval( CPulseGenerator& pulseGenerator  ,
                                       const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Run( CPulseGenerator& pulseGenerator                  ,
                      UInt32 forcedMinimalCycleDeltaInMilliSecs  = 25  ,
                      UInt32 desiredMaximumCycleDeltaInMilliSecs = 100 );

    virtual UInt32 GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    // Run() reads these on the thread of the loop, the requests come from any thread
    MT::CTAtomic< bool > m_loop;
    MT::CTAtomic< Int32 > m_immediatePulseTickets;
    MT::CTAtomic< Int32 > m_immediatePulseTicketMax;
    MT::CTAtomic< UInt32 > m_pulseDriverThreadId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-11-2004 :
        - Designed and implemented this class.

---------------------------------------------------------------------------*/
