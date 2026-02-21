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

#ifndef GUCEF_CORE_CTIMESTAMP_H
#define GUCEF_CORE_CTIMESTAMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
struct _FILETIME;
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
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDateTime;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
*  High-performance timestamp class using platform-native time storage.
*  
*  Unlike CDateTime which focuses on human-readable date/time components,
*  CTimestamp is optimized for:
*  - Maximum platform-native resolution (nanoseconds where available)
*  - Fast comparisons (single integer compare)
*  - Minimal overhead for timestamping operations
*  
*  Internal storage format:
*  - Windows: 100-nanosecond intervals since January 1, 1601 (FILETIME compatible)
*  - POSIX/Linux/Android/macOS: Nanoseconds since Unix epoch (January 1, 1970)
*  - Arduino: Microseconds since boot (no epoch, wraps ~70 minutes)
*  - Emscripten/WASM: Nanoseconds since page load
*  
*  Platform time resolution:
*  +---------------------------+---------------------------+-------------+
*  | Platform                  | API Used                  | Resolution  |
*  +---------------------------+---------------------------+-------------+
*  | Windows 8+                | GetSystemTimePreciseAs..  | 100 ns      |
*  | Windows (older)           | GetSystemTimeAsFileTime   | 100 ns      |
*  | Linux (x86/ARM)           | clock_gettime             | 1 ns        |
*  | Android                   | clock_gettime             | 1 ns        |
*  | macOS 10.12+ / iOS 10+    | clock_gettime             | 1 ns        |
*  | macOS/iOS (older)         | gettimeofday              | 1 us        |
*  | Emscripten/WASM           | emscripten_get_now        | ~1 us       |
*  | Arduino                   | micros()                  | 1 us        |
*  | Other POSIX               | gettimeofday              | 1 us        |
*  +---------------------------+---------------------------+-------------+
*  
*  Note: Timestamps are assumed to be in local time unless explicitly converted.
*  Arduino timestamps represent elapsed time since boot, not wall-clock time.
*/
class GUCEF_CORE_PUBLIC_CPP CTimestamp
{
    public:

    static const CTimestamp Empty;

    /**
     *  Returns the current local time as a high-resolution timestamp
     */
    static CTimestamp NowLocalTime( void );

    /**
     *  Returns the current UTC time as a high-resolution timestamp
     */
    static CTimestamp NowUTCTime( void );

    /**
     *  Returns the Unix epoch (January 1, 1970 00:00:00 UTC) as a timestamp
     */
    static CTimestamp UnixEpoch( void );

    /**
     *  Returns the resolution of the timestamp in nanoseconds.
     *  For example, if the platform supports 100ns resolution, returns 100.
     */
    static UInt32 GetResolutionInNanoseconds( void );

    CTimestamp( void );

    CTimestamp( const CTimestamp& src );

    explicit CTimestamp( const CDateTime& dateTime );

    explicit CTimestamp( UInt64 ticksSinceEpoch );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    explicit CTimestamp( const struct _FILETIME& fileTime );
    #endif

    ~CTimestamp();

    CTimestamp& operator=( const CTimestamp& src );

    CTimestamp& operator=( const CDateTime& dateTime );

    /**
     *  Comparison operators for CTimestamp
     */
    bool operator==( const CTimestamp& other ) const;
    bool operator!=( const CTimestamp& other ) const;
    bool operator<( const CTimestamp& other ) const;
    bool operator<=( const CTimestamp& other ) const;
    bool operator>( const CTimestamp& other ) const;
    bool operator>=( const CTimestamp& other ) const;

    /**
     *  Comparison operators for CDateTime
     */
    bool operator==( const CDateTime& other ) const;
    bool operator!=( const CDateTime& other ) const;
    bool operator<( const CDateTime& other ) const;
    bool operator<=( const CDateTime& other ) const;
    bool operator>( const CDateTime& other ) const;
    bool operator>=( const CDateTime& other ) const;

    /**
     *  Arithmetic operators for time differences
     */
    CTimestamp operator+( Int64 tickDelta ) const;
    CTimestamp operator-( Int64 tickDelta ) const;
    CTimestamp& operator+=( Int64 tickDelta );
    CTimestamp& operator-=( Int64 tickDelta );

    /**
     *  Returns the difference between two timestamps in ticks
     */
    Int64 operator-( const CTimestamp& other ) const;

    /**
     *  Convert to CDateTime (local time)
     */
    CDateTime ToDateTime( void ) const;

    /**
     *  Convert to CDateTime (UTC)
     */
    CDateTime ToDateTimeUTC( void ) const;

    /**
     *  Set from CDateTime
     */
    void FromDateTime( const CDateTime& dateTime );

    /**
     *  Returns the raw tick count (platform-specific units)
     */
    UInt64 GetTickCount( void ) const;

    /**
     *  Sets the raw tick count
     */
    void SetTickCount( UInt64 ticks );

    /**
     *  Returns the timestamp as milliseconds since epoch
     */
    UInt64 ToMillisecondsSinceEpoch( void ) const;

    /**
     *  Sets the timestamp from milliseconds since epoch
     */
    void FromMillisecondsSinceEpoch( UInt64 milliseconds );

    /**
     *  Returns the timestamp as microseconds since epoch
     */
    UInt64 ToMicrosecondsSinceEpoch( void ) const;

    /**
     *  Sets the timestamp from microseconds since epoch
     */
    void FromMicrosecondsSinceEpoch( UInt64 microseconds );

    /**
     *  Returns the timestamp as nanoseconds since epoch
     */
    UInt64 ToNanosecondsSinceEpoch( void ) const;

    /**
     *  Sets the timestamp from nanoseconds since epoch
     */
    void FromNanosecondsSinceEpoch( UInt64 nanoseconds );

    /**
     *  Returns the time difference to another timestamp in milliseconds
     */
    Int64 GetTimeDifferenceInMillisecondsTo( const CTimestamp& other ) const;

    /**
     *  Returns the time difference to another timestamp in microseconds
     */
    Int64 GetTimeDifferenceInMicrosecondsTo( const CTimestamp& other ) const;

    /**
     *  Returns whether this is a valid (non-zero) timestamp
     */
    bool IsValid( void ) const;

    /**
     *  Clears the timestamp to zero
     */
    void Clear( void );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    /**
     *  Convert to Windows FILETIME structure
     */
    struct _FILETIME ToWindowsFiletime( void ) const;

    /**
     *  Set from Windows FILETIME structure
     */
    void FromWindowsFiletime( const struct _FILETIME& fileTime );

    #endif

    private:

    /**
     *  Internal storage:
     *  - Windows: 100-nanosecond intervals since January 1, 1601 (FILETIME format)
     *  - POSIX: Nanoseconds since January 1, 1970 (Unix epoch)
     */
    UInt64 m_ticks;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTIMESTAMP_H ? */
