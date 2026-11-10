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
     *  Process wide instance which is used as the default clock
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
