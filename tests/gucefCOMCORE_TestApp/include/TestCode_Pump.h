/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
 

#ifndef CTESTCODE_PUMP_H
#define CTESTCODE_PUMP_H

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The sockets and notifiers used by the tests run on the global pulse generator. Tests that wait for something
 *  to happen pulse it from the test thread, so everything runs on one thread and no locking is needed.
 *
 *  @return true if the condition became true before the timeout
 */
template< typename Condition >
bool
PumpUntil( Condition condition, GUCEF::CORE::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::CORE::UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( condition() )
            return true;
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
    return condition();
}

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

#endif /* CTESTCODE_PUMP_H ? */
