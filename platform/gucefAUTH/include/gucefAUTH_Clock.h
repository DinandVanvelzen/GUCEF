/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CLOCK_H
#define GUCEF_AUTH_CLOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Makes the monotonic clock of the module run ahead by the given time (milliseconds, 0 for none), for a test of a login that takes
 *  minutes: the deadlines of the listener, the flows and the manager all read that clock. Wall clock times (the expiry of a token,
 *  the time of an OpenID 2.0 response nonce) are not touched. Not for anything but tests.
 */
GUCEF_AUTH_EXPORT_CPP void
SetMonotonicClockOffsetInMs( UInt64 offsetInMs );

GUCEF_AUTH_EXPORT_CPP UInt64
GetMonotonicClockOffsetInMs( void );

/** The monotonic clock of the process (CORE::CSystemMonotonicClock) plus the offset of SetMonotonicClockOffsetInMs(), in milliseconds, for deadlines */
inline UInt64
GetMonotonicTimeInMs( void )
{GUCEF_TRACE;

    return CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() + GetMonotonicClockOffsetInMs();
}

/*-------------------------------------------------------------------------*/

/** The wall clock as milliseconds since 1970-01-01 UTC, for expiry times and time stamps */
inline UInt64
GetEpochTimeInMs( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CLOCK_H ? */
