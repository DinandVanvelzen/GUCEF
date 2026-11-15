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

#include <new>

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt64
CSystemMonotonicClock::GetMonotonicTimeInMs( void ) const
{GUCEF_TRACE;

    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    if ( ticksPerMs <= 0.0 )
        return 0;
    return (UInt64) ( MT::PrecisionTickCount() / ticksPerMs );
}

/*-------------------------------------------------------------------------*/

const CSystemMonotonicClock&
CSystemMonotonicClock::Instance( void )
{GUCEF_TRACE;

    // The process wide clock is never destroyed: threads that outlive the static
    // destructors (pool threads ending their tasks while the process exits, from the
    // atexit() stop of the application) still read it. The object is constructed in
    // static storage, so it is not a heap allocation either. The class has no state.
    union TClockStorage
    {
        char bytes[ sizeof( CSystemMonotonicClock ) ];
        void* pointerAlignment;
        UInt64 integerAlignment;
        Float64 floatAlignment;
    };
    static TClockStorage storage;
    static const CSystemMonotonicClock* clock = new ( storage.bytes ) CSystemMonotonicClock();
    return *clock;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
