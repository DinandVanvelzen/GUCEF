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

#include "gucefDRN_CDRNNodePulseDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNNodePulseDriver::CDRNNodePulseDriver( const CORE::PulseGeneratorPtr& appPulseGenerator )
    : CORE::CIPulseGeneratorDriver()
    , m_appPulseGenerator( appPulseGenerator )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNodePulseDriver::~CDRNNodePulseDriver()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::Shutdown( void )
{GUCEF_TRACE;

    if ( !m_appPulseGenerator.IsNULL() )
        m_appPulseGenerator->RequestStopOfPeriodicUpdates( this );
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::RequestImmediatePulse( CORE::CPulseGenerator& /* pulseGenerator */ )
{GUCEF_TRACE;

    // never a pulse on the calling thread: the request is one for the thread of the application's pulse generator,
    // which pulses the node. A pulse generator the application pulses itself, without a driver, pulses it anyway
    if ( !m_appPulseGenerator.IsNULL() )
        m_appPulseGenerator->RequestImmediatePulseThroughDriver();
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::QueueImmediatePulse( CORE::CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    RequestImmediatePulse( pulseGenerator );
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::RequestPulsesPerImmediatePulseRequest( CORE::CPulseGenerator& /* pulseGenerator */             ,
                                                            const Int32 /* requestedPulsesPerImmediatePulseRequest */ )
{GUCEF_TRACE;

    // every pulse of the application's pulse generator is one of the node
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::RequestPeriodicPulses( CORE::CPulseGenerator& /* pulseGenerator */ ,
                                            const UInt32 pulseDeltaInMilliSecs        )
{GUCEF_TRACE;

    if ( !m_appPulseGenerator.IsNULL() )
        m_appPulseGenerator->RequestPeriodicPulses( this, pulseDeltaInMilliSecs );
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::RequestPulseInterval( CORE::CPulseGenerator& /* pulseGenerator */ ,
                                           const UInt32 pulseDeltaInMilliSecs        )
{GUCEF_TRACE;

    // the interval the requests of the node's pulse generator come to: the periodic request of this driver
    if ( !m_appPulseGenerator.IsNULL() )
        m_appPulseGenerator->RequestPeriodicPulses( this, pulseDeltaInMilliSecs );
}

/*-------------------------------------------------------------------------*/

void
CDRNNodePulseDriver::RequestStopOfPeriodicUpdates( CORE::CPulseGenerator& /* pulseGenerator */ )
{GUCEF_TRACE;

    if ( !m_appPulseGenerator.IsNULL() )
        m_appPulseGenerator->RequestStopOfPeriodicUpdates( this );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNodePulseDriver::GetPulseDriverThreadId( void ) const
{GUCEF_TRACE;

    return m_appPulseGenerator.IsNULL() ? 0 : m_appPulseGenerator->GetPulseDriverThreadId();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
