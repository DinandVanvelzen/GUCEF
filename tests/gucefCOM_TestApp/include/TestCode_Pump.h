/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_TESTCODE_PUMP_H
#define GUCEF_COM_TESTCODE_PUMP_H

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The sockets and notifiers used by the tests run on the global pulse generator. Tests that wait for something
 *  to happen pulse it from the test thread, so everything runs on one thread and no locking is needed.
 */

/*-------------------------------------------------------------------------*/

/**
 *  Pulses the global pulse generator from the test thread for the given time
 */
inline void
PumpFor( GUCEF::CORE::UInt32 durationInMs )
{GUCEF_TRACE;

    for ( GUCEF::CORE::UInt32 waited=0; waited<durationInMs; ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Pulses until the flag, which an observer sets, is true
 *
 *  @return true if the flag became true before the timeout
 */
inline bool
PumpUntilTrue( const bool& flag, GUCEF::CORE::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::CORE::UInt32 waited=0; waited<timeoutInMs && !flag; ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
    return flag;
}

/*-------------------------------------------------------------------------*/

/**
 *  Pulses until the counter, which an observer increments, reaches the target
 *
 *  @return true if the counter reached the target before the timeout
 */
inline bool
PumpUntilAtLeast( const GUCEF::CORE::UInt32& counter, GUCEF::CORE::UInt32 target, GUCEF::CORE::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::CORE::UInt32 waited=0; waited<timeoutInMs && counter < target; ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
    return counter >= target;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_TESTCODE_PUMP_H ? */
