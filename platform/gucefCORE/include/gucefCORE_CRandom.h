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

#ifndef GUCEF_CORE_CRANDOM_H
#define GUCEF_CORE_CRANDOM_H

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
 *  Cryptographically secure random number generation using the O/S provided facilities:
 *  - Linux/Android: getrandom() with /dev/urandom as fallback, Android below API level 28 has /dev/urandom only
 *  - Windows: BCryptGenRandom()
 *  All members are thread-safe.
 */
class GUCEF_CORE_PUBLIC_CPP CRandom
{
    public:

    /**
     *  Fills the buffer with the given number of random bytes
     *  @return true if all bytes were generated, false on failure (the buffer contents are then unspecified)
     */
    static bool GetRandomBytes( void* buffer, UInt32 byteCount );

    /**
     *  Gets a random value in the range [minValue, maxValue] (both inclusive) without modulo bias
     *  If minValue > maxValue the values are swapped.
     *  @return true on success, false if the O/S random source failed in which case 'result' is set to minValue
     */
    static bool GetRandomUInt32( UInt32 minValue, UInt32 maxValue, UInt32& result );

    /**
     *  Convenience variant of the above which returns minValue if the O/S random source failed.
     */
    static UInt32 GetRandomUInt32( UInt32 minValue = 0, UInt32 maxValue = GUCEF_UINT32MAX );

    /**
     *  Gets a random value in the range [0.0, 1.0)
     */
    static Float64 GetRandomFloat64( void );

    private:

    CRandom( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CRANDOM_H ? */
