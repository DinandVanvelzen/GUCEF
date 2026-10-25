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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #include <windows.h>
  #include <bcrypt.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

  #include <sys/types.h>
  #include <sys/random.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>

  // getrandom() is part of Android's C library from API level 28, below that only the device file exists.
  // Defining GUCEF_CORE_CRANDOM_NO_GETRANDOM on any platform makes it use the device file only
  #if ( !defined( GUCEF_CORE_CRANDOM_NO_GETRANDOM ) && ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) && ( !defined( __ANDROID_API__ ) || ( __ANDROID_API__ < 28 ) ) )
    #define GUCEF_CORE_CRANDOM_NO_GETRANDOM
  #endif

#endif

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

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

static bool
ReadRandomFromUrandom( UInt8* buffer, UInt32 byteCount )
{GUCEF_TRACE;

    int fd = ::open( "/dev/urandom", O_RDONLY | O_CLOEXEC );
    if ( fd < 0 )
        return false;

    bool success = true;
    while ( byteCount > 0 )
    {
        ssize_t bytesRead = ::read( fd, buffer, byteCount );
        if ( bytesRead < 0 )
        {
            if ( EINTR == errno )
                continue;
            success = false;
            break;
        }
        if ( 0 == bytesRead )
        {
            success = false;
            break;
        }
        buffer += bytesRead;
        byteCount -= (UInt32) bytesRead;
    }
    ::close( fd );
    return success;
}

#endif

/*-------------------------------------------------------------------------*/

bool
CRandom::GetRandomBytes( void* buffer, UInt32 byteCount )
{GUCEF_TRACE;

    if ( 0 == byteCount )
        return true;
    if ( GUCEF_NULL == buffer )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return 0 == BCryptGenRandom( NULL, (PUCHAR) buffer, (ULONG) byteCount, BCRYPT_USE_SYSTEM_PREFERRED_RNG );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    UInt8* bytes = static_cast< UInt8* >( buffer );
    UInt32 bytesLeft = byteCount;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Emscripten provides the random device as part of its file system, backed by the entropy of the browser or node
    return ReadRandomFromUrandom( bytes, bytesLeft );

    #elif defined( GUCEF_CORE_CRANDOM_NO_GETRANDOM )

    // The C library of this Android API level has no getrandom(), the device file is the interface
    return ReadRandomFromUrandom( bytes, bytesLeft );

    #else

    while ( bytesLeft > 0 )
    {
        ssize_t bytesRead = ::getrandom( bytes, bytesLeft, 0 );
        if ( bytesRead < 0 )
        {
            if ( EINTR == errno )
                continue;

            // getrandom is not available (old kernel, seccomp filter): fall back to the device file
            return ReadRandomFromUrandom( bytes, bytesLeft );
        }
        bytes += bytesRead;
        bytesLeft -= (UInt32) bytesRead;
    }
    return true;

    #endif /* getrandom() or the device file ? */

    #else

    // platform not supported
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CRandom::GetRandomBytes: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CRandom::GetRandomUInt32( UInt32 minValue, UInt32 maxValue, UInt32& result )
{GUCEF_TRACE;

    if ( minValue > maxValue )
    {
        UInt32 temp = minValue;
        minValue = maxValue;
        maxValue = temp;
    }

    result = minValue;
    UInt32 randomValue = 0;
    UInt32 rangeSize = maxValue - minValue + 1; // wraps to 0 for the entire 32 bit range

    if ( 0 == rangeSize )
    {
        if ( !GetRandomBytes( &randomValue, sizeof( randomValue ) ) )
            return false;
        result = randomValue;
        return true;
    }

    // Rejection sampling: discard values from the incomplete last 'bucket' to avoid modulo bias
    UInt32 limit = GUCEF_UINT32MAX - ( ( GUCEF_UINT32MAX % rangeSize ) + 1 ) % rangeSize;
    do
    {
        if ( !GetRandomBytes( &randomValue, sizeof( randomValue ) ) )
            return false;
    }
    while ( randomValue > limit );

    result = minValue + ( randomValue % rangeSize );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRandom::GetRandomUInt32( UInt32 minValue, UInt32 maxValue )
{GUCEF_TRACE;

    UInt32 result = minValue;
    GetRandomUInt32( minValue, maxValue, result );
    return result;
}

/*-------------------------------------------------------------------------*/

Float64
CRandom::GetRandomFloat64( void )
{GUCEF_TRACE;

    // 53 bits of randomness fill the mantissa of a double
    UInt64 randomValue = 0;
    if ( !GetRandomBytes( &randomValue, sizeof( randomValue ) ) )
        return 0.0;
    return ( randomValue >> 11 ) * ( 1.0 / 9007199254740992.0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
