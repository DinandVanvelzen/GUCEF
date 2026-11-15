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

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#define GUCEF_CORE_CIMONOTONICCLOCK_H

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
 *  Source of monotonic time in milliseconds.
 *  Allows time dependent logic such as rate limiting to be driven by a fake clock in tests.
 */
class GUCEF_CORE_PUBLIC_CPP CIMonotonicClock
{
    public:

    CIMonotonicClock( void ) {}

    virtual ~CIMonotonicClock() {}

    /**
     *  Milliseconds since an arbitrary but fixed point in time, never decreases
     */
    virtual UInt64 GetMonotonicTimeInMs( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Clock backed by the O/S high precision timer
 */
class GUCEF_CORE_PUBLIC_CPP CSystemMonotonicClock : public CIMonotonicClock
{
    public:

    virtual UInt64 GetMonotonicTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Process wide instance which is used as the default clock.
     *  It is never destroyed: it stays valid while the process runs its static
     *  destructors, for threads that end their work at that time.
     */
    static const CSystemMonotonicClock& Instance( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */
