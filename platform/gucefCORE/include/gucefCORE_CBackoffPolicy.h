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

#ifndef GUCEF_CORE_CBACKOFFPOLICY_H
#define GUCEF_CORE_CBACKOFFPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/**
 *  Exponential backoff for retrying failed operations.
 *  The delay of attempt n is baseDelay * factor^n, capped at maxDelay, optionally randomized by +/- jitterPercent.
 *  Not thread-safe: use one policy per retrying operation.
 */
class GUCEF_CORE_PUBLIC_CPP CBackoffPolicy
{
    public:

    /**
     *  @param baseDelayInMs    delay of the first attempt
     *  @param factor           growth per attempt, values below 1 are treated as 1 (constant delay)
     *  @param maxDelayInMs     upper bound for any delay returned, also with jitter applied
     *  @param jitterPercent    delay is randomly adjusted by up to +/- this percentage (0 = none)
     */
    CBackoffPolicy( UInt32 baseDelayInMs = 1000   ,
                    Float64 factor = 2.0          ,
                    UInt32 maxDelayInMs = 60000   ,
                    Float64 jitterPercent = 0.0   );

    /**
     *  Returns the delay to wait before the next attempt and advances the policy
     */
    UInt32 NextDelayInMs( void );

    /**
     *  Starts over from the base delay, to be used after a success
     */
    void Reset( void );

    /**
     *  Number of delays handed out since construction or the last Reset()
     */
    UInt32 GetAttemptCount( void ) const;

    private:

    UInt32 m_baseDelayInMs;
    Float64 m_factor;
    UInt32 m_maxDelayInMs;
    Float64 m_jitterPercent;
    UInt32 m_attemptCount;
    Float64 m_nextUnjitteredDelayInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBACKOFFPOLICY_H ? */
