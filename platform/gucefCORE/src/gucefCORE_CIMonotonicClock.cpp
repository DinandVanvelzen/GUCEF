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

    static const CSystemMonotonicClock clock;
    return clock;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
