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

#ifndef GUCEF_DRN_CDRNNODEPULSEDRIVER_H
#define GUCEF_DRN_CDRNNODEPULSEDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

/**
 *  The driver of the pulse generator of a CDRNNode that has no thread of its own.
 *
 *  Such a node works on a pulse generator of its own, which the node pulses under the node lock when the pulse
 *  generator of the application pulses it (CDRNNode::RunThreadCycle()): the node's channels, listeners, connectors and
 *  transports are pulsed under the node lock, as on a node thread of its own (gucefDRN design 9, lock order). What
 *  they ask of their pulse generator this driver passes on to the pulse generator of the application: periodic pulses
 *  as a periodic pulse request of the driver, immediate pulses as an immediate pulse request through the driver of the
 *  application's pulse generator. The driver never pulses on the calling thread: a request may come from any thread
 *  and with any lock held, the node lock included.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNodePulseDriver : public CORE::CIPulseGeneratorDriver
{
    public:

    explicit CDRNNodePulseDriver( const CORE::PulseGeneratorPtr& appPulseGenerator );

    virtual ~CDRNNodePulseDriver();

    /** Withdraws the periodic pulse request of this driver from the pulse generator of the application */
    void Shutdown( void );

    virtual void RequestImmediatePulse( CORE::CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulsesPerImmediatePulseRequest( CORE::CPulseGenerator& pulseGenerator                     ,
                                                        const Int32 requestedPulsesPerImmediatePulseRequest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPeriodicPulses( CORE::CPulseGenerator& pulseGenerator    ,
                                        const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulseInterval( CORE::CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestStopOfPeriodicUpdates( CORE::CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void QueueImmediatePulse( CORE::CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** The thread that drives the pulse generator of the application, which is the thread the node works on */
    virtual UInt32 GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNNodePulseDriver( const CDRNNodePulseDriver& src );              /**< not implemented */
    CDRNNodePulseDriver& operator=( const CDRNNodePulseDriver& src );   /**< not implemented */

    CORE::PulseGeneratorPtr m_appPulseGenerator;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNODEPULSEDRIVER_H ? */
