/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#include "gucefMULTIPLAY_CIMatchClock.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIMatchClock::CIMatchClock( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIMatchClock::~CIMatchClock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchSystemClock::CMatchSystemClock( void )
    : CIMatchClock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchSystemClock::~CMatchSystemClock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Int64
CMatchSystemClock::GetNowMs( void ) const
{GUCEF_TRACE;

    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

Int32
CMatchSystemClock::GetLocalMinuteOfDay( void ) const
{GUCEF_TRACE;

    CORE::CDateTime now = CORE::CDateTime::NowLocalDateTime();
    return (Int32) now.GetTime().GetHours() * 60 + (Int32) now.GetTime().GetMinutes();
}

/*-------------------------------------------------------------------------*/

CMatchSystemClock&
CMatchSystemClock::Instance( void )
{GUCEF_TRACE;

    static CMatchSystemClock clock;
    return clock;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
