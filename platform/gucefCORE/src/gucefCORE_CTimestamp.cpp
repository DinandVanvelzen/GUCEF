/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <windows.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    #include <time.h>
    #include <sys/time.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || \
      ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || \
      ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS )
    #include <time.h>
    #include <sys/time.h>
    #include <mach/mach_time.h>
    #include <Availability.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    #include <time.h>
    #include <sys/time.h>
    #include <emscripten.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    #include <Arduino.h>
#else
    #include <time.h>
    #include <sys/time.h>
#endif

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#include "gucefCORE_CTimestamp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

// Windows FILETIME epoch is January 1, 1601
// Unix epoch is January 1, 1970
// Difference in 100-nanosecond intervals
static const UInt64 WINDOWS_TICK = 10000000ULL;
static const UInt64 SEC_TO_UNIX_EPOCH = 11644473600ULL;
static const UInt64 TICKS_PER_MILLISECOND = 10000ULL;
static const UInt64 TICKS_PER_MICROSECOND = 10ULL;
static const UInt64 TICKS_PER_NANOSECOND_DIVISOR = 100ULL;  // 1 tick = 100 nanoseconds

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )

// Arduino uses microseconds since boot (no epoch)
// We store microseconds directly
static const UInt64 MICROSECONDS_PER_MILLISECOND = 1000ULL;
static const UInt64 NANOSECONDS_PER_MICROSECOND = 1000ULL;  // For conversion functions

#else

// POSIX and others use nanoseconds since Unix epoch
static const UInt64 NANOSECONDS_PER_SECOND = 1000000000ULL;
static const UInt64 NANOSECONDS_PER_MILLISECOND = 1000000ULL;
static const UInt64 NANOSECONDS_PER_MICROSECOND = 1000ULL;

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CTimestamp CTimestamp::Empty;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimestamp::CTimestamp( void )
    : m_ticks( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimestamp::CTimestamp( const CTimestamp& src )
    : m_ticks( src.m_ticks )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimestamp::CTimestamp( const CDateTime& dateTime )
    : m_ticks( 0 )
{GUCEF_TRACE;

    FromDateTime( dateTime );
}

/*-------------------------------------------------------------------------*/

CTimestamp::CTimestamp( UInt64 ticksSinceEpoch )
    : m_ticks( ticksSinceEpoch )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

CTimestamp::CTimestamp( const struct _FILETIME& fileTime )
    : m_ticks( 0 )
{GUCEF_TRACE;

    FromWindowsFiletime( fileTime );
}

#endif

/*-------------------------------------------------------------------------*/

CTimestamp::~CTimestamp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimestamp&
CTimestamp::operator=( const CTimestamp& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_ticks = src.m_ticks;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CTimestamp&
CTimestamp::operator=( const CDateTime& dateTime )
{GUCEF_TRACE;

    FromDateTime( dateTime );
    return *this;
}

/*-------------------------------------------------------------------------*/

CTimestamp
CTimestamp::NowLocalTime( void )
{GUCEF_TRACE;

    CTimestamp timestamp;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    FILETIME fileTime;
    FILETIME localFileTime;
    
    // GetSystemTimePreciseAsFileTime is available on Windows 8+
    // Fall back to GetSystemTimeAsFileTime for older Windows
    #if defined( _WIN32_WINNT ) && ( _WIN32_WINNT >= 0x0602 )
    ::GetSystemTimePreciseAsFileTime( &fileTime );
    #else
    ::GetSystemTimeAsFileTime( &fileTime );
    #endif
    
    // Convert UTC to local time
    ::FileTimeToLocalFileTime( &fileTime, &localFileTime );
    timestamp.FromWindowsFiletime( localFileTime );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    
    struct timespec ts;
    if ( clock_gettime( CLOCK_REALTIME, &ts ) == 0 )
    {
        // Store as nanoseconds since epoch
        timestamp.m_ticks = (UInt64)ts.tv_sec * NANOSECONDS_PER_SECOND + (UInt64)ts.tv_nsec;
    }

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || \
          ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || \
          ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS )
    
    // clock_gettime is available on macOS 10.12+ and iOS 10+
    #if defined( __MAC_OS_X_VERSION_MIN_REQUIRED ) && ( __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200 ) || \
        defined( __IPHONE_OS_VERSION_MIN_REQUIRED ) && ( __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 )
    struct timespec ts;
    if ( clock_gettime( CLOCK_REALTIME, &ts ) == 0 )
    {
        timestamp.m_ticks = (UInt64)ts.tv_sec * NANOSECONDS_PER_SECOND + (UInt64)ts.tv_nsec;
    }
    #else
    // Fallback to gettimeofday for older Apple platforms (microsecond resolution)
    struct timeval tv;
    if ( gettimeofday( &tv, GUCEF_NULL ) == 0 )
    {
        timestamp.m_ticks = (UInt64)tv.tv_sec * NANOSECONDS_PER_SECOND + 
                           (UInt64)tv.tv_usec * NANOSECONDS_PER_MICROSECOND;
    }
    #endif

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    
    // emscripten_get_now() returns milliseconds with sub-millisecond precision as a double
    double nowMs = emscripten_get_now();
    // Convert to nanoseconds
    timestamp.m_ticks = (UInt64)( nowMs * (double)NANOSECONDS_PER_MILLISECOND );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino micros() returns microseconds since boot (wraps around ~70 minutes)
    // Note: This is elapsed time, not wall-clock time
    timestamp.m_ticks = (UInt64)micros();

    #else
    
    // Generic fallback using gettimeofday - microsecond resolution
    struct timeval tv;
    if ( gettimeofday( &tv, GUCEF_NULL ) == 0 )
    {
        timestamp.m_ticks = (UInt64)tv.tv_sec * NANOSECONDS_PER_SECOND + 
                           (UInt64)tv.tv_usec * NANOSECONDS_PER_MICROSECOND;
    }

    #endif

    return timestamp;
}

/*-------------------------------------------------------------------------*/

CTimestamp
CTimestamp::NowUTCTime( void )
{GUCEF_TRACE;

    CTimestamp timestamp;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    FILETIME fileTime;
    
    #if defined( _WIN32_WINNT ) && ( _WIN32_WINNT >= 0x0602 )
    ::GetSystemTimePreciseAsFileTime( &fileTime );
    #else
    ::GetSystemTimeAsFileTime( &fileTime );
    #endif
    
    timestamp.FromWindowsFiletime( fileTime );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    
    struct timespec ts;
    if ( clock_gettime( CLOCK_REALTIME, &ts ) == 0 )
    {
        timestamp.m_ticks = (UInt64)ts.tv_sec * NANOSECONDS_PER_SECOND + (UInt64)ts.tv_nsec;
    }

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || \
          ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || \
          ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS )
    
    #if defined( __MAC_OS_X_VERSION_MIN_REQUIRED ) && ( __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200 ) || \
        defined( __IPHONE_OS_VERSION_MIN_REQUIRED ) && ( __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 )
    struct timespec ts;
    if ( clock_gettime( CLOCK_REALTIME, &ts ) == 0 )
    {
        timestamp.m_ticks = (UInt64)ts.tv_sec * NANOSECONDS_PER_SECOND + (UInt64)ts.tv_nsec;
    }
    #else
    struct timeval tv;
    if ( gettimeofday( &tv, GUCEF_NULL ) == 0 )
    {
        timestamp.m_ticks = (UInt64)tv.tv_sec * NANOSECONDS_PER_SECOND + 
                           (UInt64)tv.tv_usec * NANOSECONDS_PER_MICROSECOND;
    }
    #endif

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    
    // emscripten_get_now() returns milliseconds with sub-millisecond precision
    double nowMs = emscripten_get_now();
    timestamp.m_ticks = (UInt64)( nowMs * (double)NANOSECONDS_PER_MILLISECOND );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino has no concept of UTC - just use micros()
    timestamp.m_ticks = (UInt64)micros();

    #else
    
    struct timeval tv;
    if ( gettimeofday( &tv, GUCEF_NULL ) == 0 )
    {
        timestamp.m_ticks = (UInt64)tv.tv_sec * NANOSECONDS_PER_SECOND + 
                           (UInt64)tv.tv_usec * NANOSECONDS_PER_MICROSECOND;
    }

    #endif

    return timestamp;
}

/*-------------------------------------------------------------------------*/

CTimestamp
CTimestamp::UnixEpoch( void )
{GUCEF_TRACE;

    CTimestamp timestamp;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    // Unix epoch in Windows FILETIME format
    timestamp.m_ticks = SEC_TO_UNIX_EPOCH * WINDOWS_TICK;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino has no epoch concept - return 0
    timestamp.m_ticks = 0;

    #else
    
    // On POSIX, Unix epoch is tick 0
    timestamp.m_ticks = 0;

    #endif

    return timestamp;
}

/*-------------------------------------------------------------------------*/

UInt32
CTimestamp::GetResolutionInNanoseconds( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    // Windows FILETIME has 100-nanosecond resolution
    return 100;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    
    // clock_gettime has nanosecond resolution
    return 1;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || \
          ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || \
          ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS )
    
    #if defined( __MAC_OS_X_VERSION_MIN_REQUIRED ) && ( __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200 ) || \
        defined( __IPHONE_OS_VERSION_MIN_REQUIRED ) && ( __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 )
    // clock_gettime has nanosecond resolution
    return 1;
    #else
    // gettimeofday has microsecond resolution
    return 1000;
    #endif

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    
    // emscripten_get_now() typically has ~1ms resolution but can be higher
    // Returns microsecond-level precision as a double
    return 1000;  // Conservative estimate

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // micros() has microsecond resolution (actually ~4us on most boards)
    return 1000;

    #else
    
    // gettimeofday fallback has microsecond resolution
    return 1000;
    return 1000;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator==( const CTimestamp& other ) const
{GUCEF_TRACE;

    return m_ticks == other.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator!=( const CTimestamp& other ) const
{GUCEF_TRACE;

    return m_ticks != other.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator<( const CTimestamp& other ) const
{GUCEF_TRACE;

    return m_ticks < other.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator<=( const CTimestamp& other ) const
{GUCEF_TRACE;

    return m_ticks <= other.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator>( const CTimestamp& other ) const
{GUCEF_TRACE;

    return m_ticks > other.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator>=( const CTimestamp& other ) const
{GUCEF_TRACE;

    return m_ticks >= other.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator==( const CDateTime& other ) const
{GUCEF_TRACE;

    CTimestamp otherTs( other );
    return m_ticks == otherTs.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator!=( const CDateTime& other ) const
{GUCEF_TRACE;

    CTimestamp otherTs( other );
    return m_ticks != otherTs.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator<( const CDateTime& other ) const
{GUCEF_TRACE;

    CTimestamp otherTs( other );
    return m_ticks < otherTs.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator<=( const CDateTime& other ) const
{GUCEF_TRACE;

    CTimestamp otherTs( other );
    return m_ticks <= otherTs.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator>( const CDateTime& other ) const
{GUCEF_TRACE;

    CTimestamp otherTs( other );
    return m_ticks > otherTs.m_ticks;
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::operator>=( const CDateTime& other ) const
{GUCEF_TRACE;

    CTimestamp otherTs( other );
    return m_ticks >= otherTs.m_ticks;
}

/*-------------------------------------------------------------------------*/

CTimestamp
CTimestamp::operator+( Int64 tickDelta ) const
{GUCEF_TRACE;

    return CTimestamp( m_ticks + tickDelta );
}

/*-------------------------------------------------------------------------*/

CTimestamp
CTimestamp::operator-( Int64 tickDelta ) const
{GUCEF_TRACE;

    return CTimestamp( m_ticks - tickDelta );
}

/*-------------------------------------------------------------------------*/

CTimestamp&
CTimestamp::operator+=( Int64 tickDelta )
{GUCEF_TRACE;

    m_ticks += tickDelta;
    return *this;
}

/*-------------------------------------------------------------------------*/

CTimestamp&
CTimestamp::operator-=( Int64 tickDelta )
{GUCEF_TRACE;

    m_ticks -= tickDelta;
    return *this;
}

/*-------------------------------------------------------------------------*/

Int64
CTimestamp::operator-( const CTimestamp& other ) const
{GUCEF_TRACE;

    return (Int64)m_ticks - (Int64)other.m_ticks;
}

/*-------------------------------------------------------------------------*/

CDateTime
CTimestamp::ToDateTime( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    FILETIME ft = ToWindowsFiletime();
    FILETIME localFt;
    SYSTEMTIME st;
    
    ::FileTimeToLocalFileTime( &ft, &localFt );
    ::FileTimeToSystemTime( &localFt, &st );
    
    // Get local timezone offset
    TIME_ZONE_INFORMATION tzInfo;
    Int16 tzOffsetMins = 0;
    if ( TIME_ZONE_ID_INVALID != ::GetTimeZoneInformation( &tzInfo ) )
    {
        tzOffsetMins = (Int16)tzInfo.Bias;
    }
    
    return CDateTime( (Int16)st.wYear, (UInt8)st.wMonth, (UInt8)st.wDay,
                      (Int8)st.wHour, (Int8)st.wMinute, (Int8)st.wSecond,
                      (Int16)st.wMilliseconds, tzOffsetMins );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino timestamps are just elapsed microseconds, not wall-clock time
    // Return a "time" relative to 0 (epoch)
    UInt64 totalMs = m_ticks / MICROSECONDS_PER_MILLISECOND;
    UInt64 seconds = totalMs / 1000;
    UInt16 milliseconds = (UInt16)( totalMs % 1000 );
    UInt64 minutes = seconds / 60;
    seconds = seconds % 60;
    UInt64 hours = minutes / 60;
    minutes = minutes % 60;
    
    return CDateTime( 0, 0, 0,
                      (Int8)( hours % 24 ), (Int8)minutes, (Int8)seconds,
                      milliseconds, 0 );

    #else
    
    // Convert nanoseconds to seconds and remainder
    UInt64 seconds = m_ticks / NANOSECONDS_PER_SECOND;
    UInt64 nanoRemainder = m_ticks % NANOSECONDS_PER_SECOND;
    UInt16 milliseconds = (UInt16)( nanoRemainder / NANOSECONDS_PER_MILLISECOND );
    
    time_t timeVal = (time_t)seconds;
    struct tm localTm;
    localtime_r( &timeVal, &localTm );
    
    // Get timezone offset
    Int16 tzOffsetMins = 0;
    #if defined( __USE_MISC ) || defined( __USE_BSD )
    tzOffsetMins = (Int16)( localTm.tm_gmtoff / 60 );
    #endif
    
    return CDateTime( (Int16)( localTm.tm_year + 1900 ), (UInt8)( localTm.tm_mon + 1 ), (UInt8)localTm.tm_mday,
                      (Int8)localTm.tm_hour, (Int8)localTm.tm_min, (Int8)localTm.tm_sec,
                      milliseconds, tzOffsetMins );

    #endif
}

/*-------------------------------------------------------------------------*/

CDateTime
CTimestamp::ToDateTimeUTC( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    FILETIME ft = ToWindowsFiletime();
    SYSTEMTIME st;
    
    ::FileTimeToSystemTime( &ft, &st );
    
    return CDateTime( (Int16)st.wYear, (UInt8)st.wMonth, (UInt8)st.wDay,
                      (Int8)st.wHour, (Int8)st.wMinute, (Int8)st.wSecond,
                      (Int16)st.wMilliseconds, 0 );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino has no UTC concept - same as ToDateTime
    return ToDateTime();

    #else
    
    // Convert nanoseconds to seconds and remainder
    UInt64 seconds = m_ticks / NANOSECONDS_PER_SECOND;
    UInt64 nanoRemainder = m_ticks % NANOSECONDS_PER_SECOND;
    UInt16 milliseconds = (UInt16)( nanoRemainder / NANOSECONDS_PER_MILLISECOND );
    
    time_t timeVal = (time_t)seconds;
    struct tm utcTm;
    gmtime_r( &timeVal, &utcTm );
    
    return CDateTime( (Int16)( utcTm.tm_year + 1900 ), (UInt8)( utcTm.tm_mon + 1 ), (UInt8)utcTm.tm_mday,
                      (Int8)utcTm.tm_hour, (Int8)utcTm.tm_min, (Int8)utcTm.tm_sec,
                      milliseconds, 0 );

    #endif
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::FromDateTime( const CDateTime& dateTime )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    SYSTEMTIME st;
    memset( &st, 0, sizeof( st ) );
    st.wYear = (WORD)dateTime.GetYear();
    st.wMonth = (WORD)dateTime.GetMonth();
    st.wDay = (WORD)dateTime.GetDay();
    st.wHour = (WORD)dateTime.GetHours();
    st.wMinute = (WORD)dateTime.GetMinutes();
    st.wSecond = (WORD)dateTime.GetSeconds();
    st.wMilliseconds = (WORD)dateTime.GetMilliseconds();
    
    FILETIME ft;
    if ( dateTime.IsUTC() )
    {
        ::SystemTimeToFileTime( &st, &ft );
    }
    else
    {
        // Convert local time to UTC for storage
        TIME_ZONE_INFORMATION tz;
        memset( &tz, 0, sizeof( tz ) );
        tz.Bias = dateTime.GetTimeZoneUTCOffsetInMins();
        
        SYSTEMTIME utcSt;
        ::TzSpecificLocalTimeToSystemTime( &tz, &st, &utcSt );
        ::SystemTimeToFileTime( &utcSt, &ft );
    }
    
    FromWindowsFiletime( ft );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Convert time components to microseconds (no date support on Arduino)
    UInt64 totalSeconds = (UInt64)dateTime.GetHours() * 3600 +
                          (UInt64)dateTime.GetMinutes() * 60 +
                          (UInt64)dateTime.GetSeconds();
    m_ticks = totalSeconds * 1000000ULL + 
              (UInt64)dateTime.GetMilliseconds() * MICROSECONDS_PER_MILLISECOND;

    #else
    
    struct tm tmTime;
    memset( &tmTime, 0, sizeof( tmTime ) );
    tmTime.tm_year = dateTime.GetYear() - 1900;
    tmTime.tm_mon = dateTime.GetMonth() - 1;
    tmTime.tm_mday = dateTime.GetDay();
    tmTime.tm_hour = dateTime.GetHours();
    tmTime.tm_min = dateTime.GetMinutes();
    tmTime.tm_sec = dateTime.GetSeconds();
    tmTime.tm_isdst = -1;  // Let the system figure out DST
    
    time_t timeVal;
    if ( dateTime.IsUTC() )
    {
        timeVal = timegm( &tmTime );
    }
    else
    {
        timeVal = mktime( &tmTime );
    }
    
    m_ticks = (UInt64)timeVal * NANOSECONDS_PER_SECOND + 
              (UInt64)dateTime.GetMilliseconds() * NANOSECONDS_PER_MILLISECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

UInt64
CTimestamp::GetTickCount( void ) const
{GUCEF_TRACE;

    return m_ticks;
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::SetTickCount( UInt64 ticks )
{GUCEF_TRACE;

    m_ticks = ticks;
}

/*-------------------------------------------------------------------------*/

UInt64
CTimestamp::ToMillisecondsSinceEpoch( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    // Convert from Windows epoch to Unix epoch, then to milliseconds
    if ( m_ticks < SEC_TO_UNIX_EPOCH * WINDOWS_TICK )
        return 0;
    return ( m_ticks - SEC_TO_UNIX_EPOCH * WINDOWS_TICK ) / TICKS_PER_MILLISECOND;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino stores microseconds
    return m_ticks / MICROSECONDS_PER_MILLISECOND;

    #else
    
    return m_ticks / NANOSECONDS_PER_MILLISECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::FromMillisecondsSinceEpoch( UInt64 milliseconds )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    m_ticks = milliseconds * TICKS_PER_MILLISECOND + SEC_TO_UNIX_EPOCH * WINDOWS_TICK;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    m_ticks = milliseconds * MICROSECONDS_PER_MILLISECOND;

    #else
    
    m_ticks = milliseconds * NANOSECONDS_PER_MILLISECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

UInt64
CTimestamp::ToMicrosecondsSinceEpoch( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    if ( m_ticks < SEC_TO_UNIX_EPOCH * WINDOWS_TICK )
        return 0;
    return ( m_ticks - SEC_TO_UNIX_EPOCH * WINDOWS_TICK ) / TICKS_PER_MICROSECOND;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino stores microseconds directly
    return m_ticks;

    #else
    
    return m_ticks / NANOSECONDS_PER_MICROSECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::FromMicrosecondsSinceEpoch( UInt64 microseconds )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    m_ticks = microseconds * TICKS_PER_MICROSECOND + SEC_TO_UNIX_EPOCH * WINDOWS_TICK;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    m_ticks = microseconds;

    #else
    
    m_ticks = microseconds * NANOSECONDS_PER_MICROSECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

UInt64
CTimestamp::ToNanosecondsSinceEpoch( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    if ( m_ticks < SEC_TO_UNIX_EPOCH * WINDOWS_TICK )
        return 0;
    return ( m_ticks - SEC_TO_UNIX_EPOCH * WINDOWS_TICK ) * TICKS_PER_NANOSECOND_DIVISOR;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino stores microseconds, convert to nanoseconds
    return m_ticks * NANOSECONDS_PER_MICROSECOND;

    #else
    
    return m_ticks;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::FromNanosecondsSinceEpoch( UInt64 nanoseconds )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    m_ticks = nanoseconds / TICKS_PER_NANOSECOND_DIVISOR + SEC_TO_UNIX_EPOCH * WINDOWS_TICK;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino stores microseconds
    m_ticks = nanoseconds / NANOSECONDS_PER_MICROSECOND;

    #else
    
    m_ticks = nanoseconds;

    #endif
}

/*-------------------------------------------------------------------------*/

Int64
CTimestamp::GetTimeDifferenceInMillisecondsTo( const CTimestamp& other ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    return ( (Int64)other.m_ticks - (Int64)m_ticks ) / (Int64)TICKS_PER_MILLISECOND;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    return ( (Int64)other.m_ticks - (Int64)m_ticks ) / (Int64)MICROSECONDS_PER_MILLISECOND;

    #else
    
    return ( (Int64)other.m_ticks - (Int64)m_ticks ) / (Int64)NANOSECONDS_PER_MILLISECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

Int64
CTimestamp::GetTimeDifferenceInMicrosecondsTo( const CTimestamp& other ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    return ( (Int64)other.m_ticks - (Int64)m_ticks ) / (Int64)TICKS_PER_MICROSECOND;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ARDUINO )
    
    // Arduino stores microseconds directly
    return (Int64)other.m_ticks - (Int64)m_ticks;

    #else
    
    return ( (Int64)other.m_ticks - (Int64)m_ticks ) / (Int64)NANOSECONDS_PER_MICROSECOND;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CTimestamp::IsValid( void ) const
{GUCEF_TRACE;

    return m_ticks != 0;
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::Clear( void )
{GUCEF_TRACE;

    m_ticks = 0;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

struct _FILETIME
CTimestamp::ToWindowsFiletime( void ) const
{GUCEF_TRACE;

    ULARGE_INTEGER uli;
    uli.QuadPart = m_ticks;
    
    FILETIME ft;
    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;
    return ft;
}

/*-------------------------------------------------------------------------*/

void
CTimestamp::FromWindowsFiletime( const struct _FILETIME& fileTime )
{GUCEF_TRACE;

    ULARGE_INTEGER uli;
    uli.LowPart = fileTime.dwLowDateTime;
    uli.HighPart = fileTime.dwHighDateTime;
    m_ticks = uli.QuadPart;
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
