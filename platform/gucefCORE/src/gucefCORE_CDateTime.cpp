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

#include <time.h>
#include <string.h>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "gucefCORE_CDateTime.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #include <Windows.h>

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    #include <climits>

    #define sscanf_s    sscanf
    #define sprintf_s( s, ss, f, ... )   sprintf( s, f, __VA_ARGS__ )

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CDateTime CDateTime::Empty;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

class GUCEF_CORE_PRIVATE_CPP COSDateTimeUtils
{
    public:

    static UInt16 GetTimezoneOffsetInMins( void )
    {
        ::TIME_ZONE_INFORMATION tzInfo;
        memset( &tzInfo, 0, sizeof( tzInfo ) );
        if ( TIME_ZONE_ID_INVALID != ::GetTimeZoneInformation( &tzInfo ) )
        {
            return (Int16) tzInfo.Bias;
        }
        return 0;
    }

    static void DateTimeToWin32SystemTime( const CDateTime& datetime, SYSTEMTIME& systemTime )
    {
        systemTime.wYear = (WORD) datetime.GetYear();
        systemTime.wMonth = (WORD) datetime.GetMonth();
        systemTime.wDay = (WORD) datetime.GetDay();
        systemTime.wHour = (WORD) datetime.GetHours();
        systemTime.wMinute = (WORD) datetime.GetMinutes();
        systemTime.wSecond = (WORD) datetime.GetSeconds();
        systemTime.wMilliseconds = (WORD) datetime.GetMilliseconds();
    }

    static bool DateTimeToWin32FileTime( const CDateTime& datetime, FILETIME& fileTime )
    {
        ::SYSTEMTIME systemTime;
        memset( &systemTime, 0, sizeof( systemTime ) );
        DateTimeToWin32SystemTime( datetime, systemTime );

        if ( datetime.IsUTC() )
        {
            return ::SystemTimeToFileTime( &systemTime, &fileTime ) == TRUE;
        }
        else
        {
            ::TIME_ZONE_INFORMATION tz;
            memset( &tz, 0, sizeof( tz ) );
            tz.Bias = datetime.GetTimeZoneUTCOffsetInMins();

            ::SYSTEMTIME utcTime;
            memset( &utcTime, 0, sizeof( utcTime ) );
            if ( ::TzSpecificLocalTimeToSystemTime( &tz, &systemTime, &utcTime ) == TRUE )
                return ::SystemTimeToFileTime( &utcTime, &fileTime ) == TRUE;
            else
                return false;
        }
    }

    static void Win32SystemTimeToDateTime( SYSTEMTIME& systemTime, UInt16 timezoneOffsetInMins, CDateTime& datetime )
    {
        datetime = CDateTime( (Int16) systemTime.wYear          ,
                              (UInt8) systemTime.wMonth         ,
                              (UInt8) systemTime.wDay           ,
                              (UInt8) systemTime.wHour          ,
                              (UInt8) systemTime.wMinute        ,
                              (UInt8) systemTime.wSecond        ,
                              (UInt16) systemTime.wMilliseconds ,
                              timezoneOffsetInMins              );
    }

    static void Win32FileTimeToDateTime( const FILETIME& fileTime, CDateTime& datetime )
    {
        ::SYSTEMTIME systemTime;
        memset( &systemTime, 0, sizeof( systemTime ) );
        ::FileTimeToSystemTime( &fileTime, &systemTime );

        Win32SystemTimeToDateTime( systemTime, 0, datetime );

        DateTimeToWin32SystemTime( datetime, systemTime );
    }

    static CDateTime GetFutureMaxAsDateTime( void )
    {
        ::ULARGE_INTEGER fileTimeUInt64;
        fileTimeUInt64.QuadPart = 0x7fff35f4f06c58f0;
        ::FILETIME filetime;
        filetime.dwHighDateTime = fileTimeUInt64.HighPart;
        filetime.dwLowDateTime = fileTimeUInt64.LowPart;

        CDateTime dtFormat;
        Win32FileTimeToDateTime( filetime, dtFormat );
        return dtFormat;
    }

    static CDateTime GetPastMaxAsDateTime( void )
    {
        ::ULARGE_INTEGER fileTimeUInt64;
        fileTimeUInt64.QuadPart = 0;
        ::FILETIME filetime;
        filetime.dwHighDateTime = fileTimeUInt64.HighPart;
        filetime.dwLowDateTime = fileTimeUInt64.LowPart;

        CDateTime dtFormat;
        Win32FileTimeToDateTime( filetime, dtFormat );
        return dtFormat;
    }

    static CDateTime DateTimeToUtc( const CDateTime& datetime )
    {
        ::SYSTEMTIME systemTime;
        memset( &systemTime, 0, sizeof( systemTime ) );
        DateTimeToWin32SystemTime( datetime, systemTime );

        ::TIME_ZONE_INFORMATION tz;
        memset( &tz, 0, sizeof( tz ) );
        tz.Bias = datetime.GetTimeZoneUTCOffsetInMins();

        ::SYSTEMTIME utcTime;
        memset( &utcTime, 0, sizeof( utcTime ) );
        ::TzSpecificLocalTimeToSystemTime( &tz, &systemTime, &utcTime );

        CDateTime utcDatetime;
        Win32SystemTimeToDateTime( utcTime, 0, utcDatetime );
        return utcDatetime;
    }

    // -1 = First datetime is earlier than second datetime.
    //  0 = First datetime is equal to second datetime.
    //  1 = First datetime is later than second datetime.
    static Int32 CompareDateTime( const CDateTime& datetimeA, const CDateTime& datetimeB )
    {
        ::FILETIME fileTimeA;
        memset( &fileTimeA, 0, sizeof( fileTimeA ) );
        DateTimeToWin32FileTime( datetimeA, fileTimeA );

        ::FILETIME fileTimeB;
        memset( &fileTimeB, 0, sizeof( fileTimeB ) );
        DateTimeToWin32FileTime( datetimeB, fileTimeB );

        return ::CompareFileTime( &fileTimeA, &fileTimeB );
    }

    static Int64 GetTimeDifferenceInMillisecondsFromAtowardsB( const CDateTime& datetimeA, const CDateTime& datetimeB )
    {
        ::FILETIME fileTimeA;
        DateTimeToWin32FileTime( datetimeA, fileTimeA );

        ::FILETIME fileTimeB;
        DateTimeToWin32FileTime( datetimeB, fileTimeB );

        ULARGE_INTEGER fileTimeUInt64A;
        fileTimeUInt64A.LowPart = fileTimeA.dwLowDateTime;
        fileTimeUInt64A.HighPart = fileTimeA.dwHighDateTime;

        ULARGE_INTEGER fileTimeUInt64B;
        fileTimeUInt64B.LowPart = fileTimeB.dwLowDateTime;
        fileTimeUInt64B.HighPart = fileTimeB.dwHighDateTime;

        Int64 quadDiff = fileTimeUInt64B.QuadPart - fileTimeUInt64A.QuadPart;
        quadDiff = ( quadDiff / 10000 );
        return quadDiff;
    }

    static CDateTime NowLocalDateTime( void )
    {
        ::SYSTEMTIME utcSystemTime;
        memset( &utcSystemTime, 0, sizeof( utcSystemTime ) );
        ::GetSystemTime( &utcSystemTime );
        ::SYSTEMTIME localSystemTime;
        memset( &localSystemTime, 0, sizeof( localSystemTime ) );
        ::SystemTimeToTzSpecificLocalTime( NULL, &utcSystemTime, &localSystemTime );

        CDateTime localDatetime;
        Win32SystemTimeToDateTime( localSystemTime, GetTimezoneOffsetInMins(), localDatetime );
        return localDatetime;
    }

    static CDateTime NowUTCDateTime( void )
    {
        ::SYSTEMTIME systemTime;
        memset( &systemTime, 0, sizeof( systemTime ) );
        ::GetSystemTime( &systemTime );

        CDateTime utcDatetime;
        Win32SystemTimeToDateTime( systemTime, 0, utcDatetime );
        return utcDatetime;
    }

    static void SetFromUnixEpochBasedTicksInMillisecs( UInt64 unixDtInMsTicks, CDateTime& dt )
    {
        // Convert to FILETIME compatible resolution (ms to nanos) and compensate for different epoch offset
        UINT64 filettimeDt = ( 10000 * unixDtInMsTicks ) + 116444736000000000;

        ULARGE_INTEGER memoryAllignedUInt64;
        memoryAllignedUInt64.QuadPart = filettimeDt;

        ::FILETIME filetime;
        filetime.dwLowDateTime = memoryAllignedUInt64.LowPart;
        filetime.dwHighDateTime = memoryAllignedUInt64.HighPart;

        Win32FileTimeToDateTime( filetime, dt );
    }

    static UInt64 ToUnixEpochBasedTicksInMillisecs( const CDateTime& datetime )
    {
        ::FILETIME fileTime;
        memset( &fileTime, 0, sizeof( fileTime ) );
        DateTimeToWin32FileTime( datetime, fileTime );

        ULARGE_INTEGER memoryAllignedUInt64;
        memoryAllignedUInt64.LowPart = fileTime.dwLowDateTime;
        memoryAllignedUInt64.HighPart = fileTime.dwHighDateTime;

        // Convert from FILETIME compatible resolution (nanos to ms) and compensate for different epoch offset
        UInt64 unixDtInMsTicks = ( memoryAllignedUInt64.QuadPart - 116444736000000000 ) / 10000;
        return unixDtInMsTicks;
    }
};

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

class GUCEF_CORE_PRIVATE_CPP COSDateTimeUtils
{
    public:

    static UInt16 GetTimezoneOffsetInMins( void )
    {
        struct timespec time;
        ::memset( &time, 0, sizeof(time) );
        ::clock_gettime( CLOCK_REALTIME, &time );
        struct tm hereTime;
        ::localtime_r( &time.tv_sec, &hereTime );
        return hereTime.tm_gmtoff / 60;
    }

    static void DateTimeToTm( const CDateTime& datetime, struct ::tm& time )
    {
        ::memset( &time, 0, sizeof(time) );
        time.tm_year = datetime.GetYear() - 1900;
        time.tm_mon = datetime.GetMonth() - 1;
        time.tm_mday = datetime.GetDay() - 1;
        time.tm_hour = datetime.GetHours();
        time.tm_min = datetime.GetMinutes();
        time.tm_sec = datetime.GetSeconds();
        time.tm_gmtoff = datetime.GetTimeZoneUTCOffsetInMins() * 60;
    }

    static void DateTimeToTimespec( const CDateTime& datetime, struct ::timespec& tspec )
    {
        struct ::tm time;
        DateTimeToTm( datetime, time );  // <- this converts keeping the timezone intact with gmtoff

        tspec.tv_sec = ::timegm( &time ) - time.tm_gmtoff; // <- this converts to UTC
        tspec.tv_nsec = datetime.GetMilliseconds() * 1000000;
    }

    static void TimespecToDateTime( const struct ::timespec& tspec, CDateTime& datetime )
    {
        struct tm utcTime;
        ::gmtime_r( &tspec.tv_sec, &utcTime );
        datetime.Set( (Int16) utcTime.tm_year+1900,
                      (UInt8) utcTime.tm_mon+1,
                      (UInt8) utcTime.tm_mday+1,
                      (UInt8) utcTime.tm_hour,
                      (UInt8) utcTime.tm_min,
                      (UInt8) utcTime.tm_sec,
                      (UInt16) ( tspec.tv_nsec / 1000000 ),
                      0 );
    }

    static CDateTime NowLocalDateTime( void )
    {
        struct timespec time;
        ::memset( &time, 0, sizeof(time) );
        ::clock_gettime( CLOCK_REALTIME, &time );
        struct tm hereTime;
        ::localtime_r( &time.tv_sec, &hereTime );
        return CDateTime( (Int16) hereTime.tm_year+1900,
                          (UInt8) hereTime.tm_mon+1,
                          (UInt8) hereTime.tm_mday+1,
                          (UInt8) hereTime.tm_hour,
                          (UInt8) hereTime.tm_min,
                          (UInt8) hereTime.tm_sec,
                          (UInt16) ( time.tv_nsec / 1000000 ),
                          hereTime.tm_gmtoff / 60 );
    }

    static CDateTime NowUTCDateTime( void )
    {
        struct timespec time;
        ::memset( &time, 0, sizeof(time) );
        ::clock_gettime( CLOCK_REALTIME, &time );
        struct tm utcTime;
        ::gmtime_r( &time.tv_sec, &utcTime );
        return CDateTime( (Int16) utcTime.tm_year+1900,
                          (UInt8) utcTime.tm_mon+1,
                          (UInt8) utcTime.tm_mday+1,
                          (UInt8) utcTime.tm_hour,
                          (UInt8) utcTime.tm_min,
                          (UInt8) utcTime.tm_sec,
                          (UInt16) ( time.tv_nsec / 1000000 ),
                          0 );
    }

    static CDateTime DateTimeToUtc( const CDateTime& datetime )
    {
        // Converting to timespec also converts to UTC by turning it into a UTC based offset
        struct ::timespec tspec;
        DateTimeToTimespec( datetime, tspec );

        CDateTime utcDatetime;
        TimespecToDateTime( tspec, utcDatetime );
        return utcDatetime;
    }

    // -1 = First datetime is earlier than second datetime.
    //  0 = First datetime is equal to second datetime.
    //  1 = First datetime is later than second datetime.
    static Int32 CompareDateTime( const CDateTime& datetimeA, const CDateTime& datetimeB )
    {
        // Converting to timespec also converts to UTC by turning it into a UTC based offset
        // This allows for easy comparison

        struct ::timespec tspecA;
        DateTimeToTimespec( datetimeA, tspecA );

        struct ::timespec tspecB;
        DateTimeToTimespec( datetimeB, tspecB );

        if ( tspecA.tv_sec > tspecB.tv_sec )
            return 1;
        if ( tspecA.tv_sec < tspecB.tv_sec )
            return -1;

        // seconds are the same, but they can still differ on the remainder nanoseconds
        if ( tspecA.tv_nsec > tspecB.tv_nsec )
            return 1;
        if ( tspecA.tv_nsec < tspecB.tv_nsec )
            return -1;
        return 0;
    }

    static Int64 GetTimeDifferenceInMillisecondsFromAtowardsB( const CDateTime& datetimeA, const CDateTime& datetimeB )
    {
        // Converting to timespec also converts to UTC by turning it into a UTC based offset
        // This allows for easy subtraction

        struct ::timespec tspecA;
        DateTimeToTimespec( datetimeA, tspecA );

        struct ::timespec tspecB;
        DateTimeToTimespec( datetimeB, tspecB );

        Int64 deltaSecs = tspecA.tv_sec - tspecB.tv_sec;
        Int64 deltaNanoSecs = tspecA.tv_nsec - tspecB.tv_nsec;
        return ( deltaSecs * 1000 ) + ( deltaNanoSecs / 1000000 );
    }

    static void SetFromUnixEpochBasedTicksInMillisecs( UInt64 unixDtInMsTicks, CDateTime& dt )
    {
        // Break the milliseconds total into seconds plus the nanoseconds remainder
        // This allows us to use the timespec conversion function
        struct timespec time;
        time.tv_sec = (time_t) unixDtInMsTicks / 1000;
        time.tv_nsec = ( unixDtInMsTicks - ( time.tv_sec * 1000 ) ) * 1000000;

        TimespecToDateTime( time, dt );
    }

    static UInt64 ToUnixEpochBasedTicksInMillisecs( const CDateTime& datetime )
    {
        struct timespec time;
        DateTimeToTimespec( datetime, time );

        UInt64 unixDtInMsTicks = ( time.tv_sec * 1000 ) + (UInt64)( time.tv_nsec / 1000000 );
        return unixDtInMsTicks;
    }

    static CDateTime GetFutureMaxAsDateTime( void )
    {
        struct timespec time;
        time.tv_sec = (time_t) 0;
        time.tv_sec -= 1; // intentionally wrap around the unsigned
        time.tv_nsec = LONG_MAX;

        CDateTime dtFormat;
        TimespecToDateTime( time, dtFormat );
        return dtFormat;
    }

    static CDateTime GetPastMaxAsDateTime( void )
    {
        struct timespec time;
        time.tv_sec = (time_t) 0;
        time.tv_nsec = 0;

        CDateTime dtFormat;
        TimespecToDateTime( time, dtFormat );
        return dtFormat;
    }
};

#else

class GUCEF_CORE_PRIVATE_CPP COSDateTimeUtils
{
    public:

    static UInt16 GetTimezoneOffsetInMins( void )
    {
        return 0;
    }

    static CDateTime NowLocalDateTime( void )
    {
        return CDateTime();
    }

    static CDateTime NowUTCDateTime( void )
    {
        return CDateTime();
    }

    static CDateTime DateTimeToUtc( const CDateTime& datetime )
    {
        return datetime;
    }

    static Int32 CompareDateTime( const CDateTime& datetimeA, const CDateTime& datetimeB )
    {
        return 0;
    }

    static Int64 SubtractBFromAndGetTimeDifferenceInMilliseconds( const CDateTime& datetimeA, const CDateTime& datetimeB )
    {
        return 0;
    }

    static UInt64 ToUnixEpochBasedTicksInMillisecs( const CDateTime& datetime )
    {
        return 0;
    }

    static CDateTime GetFutureMaxAsDateTime( void )
    {
        return CDateTime( GUCEF_MT_INT16MAX, UInt8( 12 ), UInt8( 31 ), UInt8( 23 ), UInt8( 59 ), UInt8( 59 ), UInt16( 999 ) );
    }

    static CDateTime GetPastMaxAsDateTime( void )
    {
        return CDateTime( GUCEF_MT_INT16MIN, UInt8( 0 ), UInt8( 0 ), UInt8( 0 ), UInt8( 0 ), UInt8( 0 ), UInt16( 0 ) );
    }
};

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CDateTime CDateTime::PastMax = COSDateTimeUtils::GetPastMaxAsDateTime();
const CDateTime CDateTime::FutureMax = COSDateTimeUtils::GetFutureMaxAsDateTime();

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDateTime::CDateTime( const time_t src, bool isUtc )
    : CDate( src, isUtc )
    , CTime( src, isUtc )
    , m_timezoneOffsetInMins( 0 )
{GUCEF_TRACE;

    if ( !isUtc )
        m_timezoneOffsetInMins = COSDateTimeUtils::GetTimezoneOffsetInMins();
}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( const struct tm* tmStruct, bool isUtc )
    : CDate()
    , CTime()
    , m_timezoneOffsetInMins( 0 )
{GUCEF_TRACE;

    FromTmStruct( tmStruct, isUtc );
}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( const timespec& timespecStruct, bool isUtc )
    : CDate( timespecStruct.tv_sec, isUtc )
    , CTime( timespecStruct.tv_sec, isUtc )
    , m_timezoneOffsetInMins( 0 )
{GUCEF_TRACE;

    // Convert nano seconds to milliseconds which is the max resolution we can store
    SetMilliseconds( (UInt16) ( timespecStruct.tv_nsec / 1000000 ) );

    if ( !isUtc )
        m_timezoneOffsetInMins = COSDateTimeUtils::GetTimezoneOffsetInMins();
}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( const CTime& src, bool isUtc )
    : CDate()
    , CTime( src )
    , m_timezoneOffsetInMins( 0 )
{GUCEF_TRACE;

    if ( !isUtc )
        m_timezoneOffsetInMins = COSDateTimeUtils::GetTimezoneOffsetInMins();
}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( const CDate& src, bool isUtc )
    : CDate( src )
    , CTime()
    , m_timezoneOffsetInMins( 0 )
{GUCEF_TRACE;

    if ( !isUtc )
        m_timezoneOffsetInMins = COSDateTimeUtils::GetTimezoneOffsetInMins();
}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( const CDate& dateSrc, const CTime& timeSrc, Int16 timezoneOffsetInMins )
    : CDate( dateSrc )
    , CTime( timeSrc)
    , m_timezoneOffsetInMins( timezoneOffsetInMins )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( const CDateTime& src )
    : CDate( src )
    , CTime( src )
    , m_timezoneOffsetInMins( src.m_timezoneOffsetInMins )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTime::CDateTime( Int16 year                 ,
                      UInt8 month                ,
                      UInt8 day                  ,
                      Int8 hours                 ,
                      Int8 minutes               ,
                      Int8 seconds               , 
                      Int16 milliseconds         ,
                      Int16 timezoneOffsetInMins )
    : CDate( year, month, day )
    , CTime( hours, minutes, seconds, milliseconds )
    , m_timezoneOffsetInMins( timezoneOffsetInMins )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDateTime::Set( Int16 year                 ,
                UInt8 month                ,
                UInt8 day                  ,
                Int8 hours                 ,  
                Int8 minutes               ,
                Int8 seconds               ,
                Int16 milliseconds         ,
                Int16 timezoneOffsetInMins )
{GUCEF_TRACE;

    m_year = year;
    m_month = month;
    m_day = day;
    m_hours = hours;
    m_minutes = minutes;
    m_seconds = seconds;
    m_milliseconds = milliseconds;
    m_timezoneOffsetInMins = timezoneOffsetInMins;
}

/*-------------------------------------------------------------------------*/

UInt64
CDateTime::ToUnixEpochBasedTicksInMillisecs( void ) const
{GUCEF_TRACE;

    return COSDateTimeUtils::ToUnixEpochBasedTicksInMillisecs( *this );
}

/*-------------------------------------------------------------------------*/

void
CDateTime::FromUnixEpochBasedTicksInMillisecs( UInt64 unixDtInMsTicks )
{GUCEF_TRACE;

    COSDateTimeUtils::SetFromUnixEpochBasedTicksInMillisecs( unixDtInMsTicks, *this );
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

CDateTime::CDateTime( const struct _FILETIME& src )
    : CDate()
    , CTime()
    , m_timezoneOffsetInMins( 0 )
{GUCEF_TRACE;

    COSDateTimeUtils::Win32FileTimeToDateTime( src, *this );
}

/*-------------------------------------------------------------------------*/

struct _FILETIME 
CDateTime::ToWindowsFiletime( void ) const
{GUCEF_TRACE;

    FILETIME fileTime;
    memset( &fileTime, 0, sizeof fileTime );
    COSDateTimeUtils::DateTimeToWin32FileTime( *this, fileTime );
    return fileTime;
}

#endif

/*-------------------------------------------------------------------------*/

CDateTime::~CDateTime()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTime
CDateTime::CompileDateTime( const char* __date__Macro ,
                            const char* __time__Macro )
{GUCEF_TRACE;

    // __DATE__ is a preprocessor macro that expands to current date (at compile time) in the form mmm dd yyyy (e.g. "Jan 14 2012"), as a string. The __DATE__ macro can be used to provide information about the particular moment a binary was built.
    // __TIME__ is a preprocessor macro that expands to current time (at compile time) in the form hh:mm:ss in 24 hour time (e.g. "22:29:12"), as a string. The __TIME__ macro can be used to provide information about the particular moment a binary was built.

    CORE::CDateTime dt;

    CORE::Int32 month=0, day=0, year=0;
    char month3Letter[ 5 ] = { '\0', '\0', '\0', '\0', '\0' };
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    CORE::Int32 hour=0, minutes=0, seconds=0;

    sscanf( __date__Macro, "%s %d %d", month3Letter, &day, &year );
    month = ( ( (CORE::Int32)( strstr( month_names, month3Letter ) - month_names ) ) / 3 ) + 1;
    sscanf( __time__Macro, "%d%*c%d%*c%d", &hour, &minutes, &seconds );

    dt.Set( (CORE::Int16) year   ,
            (CORE::UInt8) month  ,
            (CORE::UInt8) day    ,
            (CORE::Int8) hour    ,
            (CORE::Int8) minutes ,
            (CORE::Int8) seconds );

    return dt;
}

/*-------------------------------------------------------------------------*/

CDateTime
CDateTime::NowUTCDateTime( void )
{GUCEF_TRACE;

    return COSDateTimeUtils::NowUTCDateTime();
}

/*-------------------------------------------------------------------------*/

CDateTime
CDateTime::NowLocalDateTime( void )
{GUCEF_TRACE;

    return COSDateTimeUtils::NowLocalDateTime();
}

/*-------------------------------------------------------------------------*/

CDateTime
CDateTime::UnixEpochDateTime( void )
{GUCEF_TRACE;

    return CDateTime( 1970, 1, 1 );
}

/*-------------------------------------------------------------------------*/

CDateTime
CDateTime::WindowsNTEpochDateTime( void )
{GUCEF_TRACE;

    return CDateTime( 1601, 1, 1 );
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::operator>( const CDateTime& other ) const
{GUCEF_TRACE;

    return COSDateTimeUtils::CompareDateTime( *this, other ) > 0;
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::operator<( const CDateTime& other ) const
{GUCEF_TRACE;

    return COSDateTimeUtils::CompareDateTime( *this, other ) < 0;
}

/*-------------------------------------------------------------------------*/

CDateTime&
CDateTime::operator=( const CDateTime& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CDate::operator=( src );
        CTime::operator=( src );
        m_timezoneOffsetInMins = src.m_timezoneOffsetInMins;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::operator==( const CDateTime& other ) const
{GUCEF_TRACE;

    return 0 == COSDateTimeUtils::CompareDateTime( *this, other );
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::operator!=( const CDateTime& other ) const
{GUCEF_TRACE;

    return 0 != COSDateTimeUtils::CompareDateTime( *this, other );
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::operator>=( const CDateTime& other ) const
{GUCEF_TRACE;

    return (*this) > other || (*this) == other;
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::operator<=( const CDateTime& other ) const
{GUCEF_TRACE;

    return (*this) < other || (*this) == other;
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::Compare( const CDateTime& other ) const
{GUCEF_TRACE;

    return COSDateTimeUtils::CompareDateTime( *this, other );
}

/*-------------------------------------------------------------------------*/

void 
CDateTime::SetTimeZoneUTCOffsetInMins( Int16 tzOffsetInMins )
{GUCEF_TRACE;

    m_timezoneOffsetInMins = tzOffsetInMins;
}

/*-------------------------------------------------------------------------*/

Int16
CDateTime::GetTimeZoneUTCOffsetInMins( void ) const
{GUCEF_TRACE;

    return m_timezoneOffsetInMins;
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::IsUTC( void ) const
{GUCEF_TRACE;

    return 0 == m_timezoneOffsetInMins;
}

/*-------------------------------------------------------------------------*/

CDateTime
CDateTime::ToUTC( void ) const
{GUCEF_TRACE;

    if ( 0 != m_timezoneOffsetInMins )
        return COSDateTimeUtils::DateTimeToUtc( *this );
    return *this;
}

/*-------------------------------------------------------------------------*/

Int64
CDateTime::GetTimeDifferenceInMillisecondsTowards( const CDateTime& other ) const
{GUCEF_TRACE;

    return COSDateTimeUtils::GetTimeDifferenceInMillisecondsFromAtowardsB( *this, other );
}

/*-------------------------------------------------------------------------*/

Int64
CDateTime::GetTimeDifferenceInMillisecondsToNow( void ) const
{GUCEF_TRACE;

    return COSDateTimeUtils::GetTimeDifferenceInMillisecondsFromAtowardsB( *this, CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------*/

CDate&
CDateTime::GetDate( void )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

const CDate&
CDateTime::GetDate( void ) const
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

CTime&
CDateTime::GetTime( void )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

const CTime&
CDateTime::GetTime( void ) const
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

// Returns the nr of bytes occupied by the timezone offset description 
// the actual timezone offset is stored in the tzOffset output parameter
// if no timezone offset is found, the tzOffset is set to 0 and -1 is returned
Int32
TimezoneOffsetInMinsFromIso8601DateTimeStringRemnant( const char* sourceBuffer, UInt32 sourceBufferSize, Int16& tzOffset )
{GUCEF_TRACE;

    tzOffset = 0;

    // We are looking for Z or +hh:mm or -hh:mm
    if ( sourceBufferSize > 0 )
    {
        if ( *sourceBuffer == '\0' )
        {
            tzOffset = 0;
            return 0;
        }
        if ( *sourceBuffer == 'Z' || *sourceBuffer == '\0' )
        {
            tzOffset = 0;
            return 1;
        }
        if ( sourceBufferSize > 5 )
        {
            Int32 hours = 0;
            Int32 minutes = 0;
            int readParts = sscanf_s( sourceBuffer+1, "%02d%*c%02d", &hours, &minutes );
            if ( readParts == 2 )
            {
                minutes += hours * 60;
                if ( *sourceBuffer == '-' )
                    minutes = -1 * minutes;

                tzOffset = (Int16) minutes;
                return 6;
            }
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
TimezoneOffsetInMinsFromIso8601DateTimeStringParts( const GUCEF::vector< Int32 >& dtParts , 
                                                    bool isNegativeTzOffset             ,
                                                    Int16& tzOffset                     )
{GUCEF_TRACE;

    tzOffset = 0;
    if ( dtParts.size() >= 2 )
    {
        Int32 minutes = dtParts[ dtParts.size()-1 ];
        Int32 hours = dtParts[ dtParts.size()-2 ];

        tzOffset = (Int16) ( minutes + ( hours * 60 ) );

        if ( isNegativeTzOffset )
            tzOffset = -1 * tzOffset;
    
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

inline bool
IsDigit( char ch ) {GUCEF_TRACE; return ( ch >= '0' && ch <= '9' ); }

/*-------------------------------------------------------------------------*/

inline bool
HasNonDigitDotT( const char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    if ( bufferSize > 0 && GUCEF_NULL != buffer )
    {    
        // check for last index. depends on if we have a null terminator
        UInt32 lastCharIndex = 0;
        if ( '\0' == buffer[ bufferSize-1 ] )
        {
            if ( bufferSize > 1 )
                lastCharIndex = bufferSize-2;
        }
        else
            lastCharIndex = bufferSize-1;

        for ( UInt32 i=0; i<bufferSize; ++i )
        {
            if ( !IsDigit( buffer[ i ] ) && buffer[ i ] != '.' && buffer[ i ] != 'T' && buffer[ i ] != '\0' )
            {
                if ( i == lastCharIndex )
                {
                    if ( buffer[ i ] == 'Z' )
                        return false;
                }
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::FromIso8601DateTimeString( const void* sourceBuffer, UInt32 sourceBufferSize )
{GUCEF_TRACE;

    // Reset content
    Set();

    // initial input sanity check
    if ( sourceBufferSize < 4 || GUCEF_NULL == sourceBuffer )
        return -1;

    const char* dtBuffer = static_cast< const char* >( sourceBuffer );
    Int16 tzOffset=0;

    if ( HasNonDigitDotT( dtBuffer, sourceBufferSize ) )
    {
        // If we get here we have seperators in the string
        // This makes it clearer which digits belong to which part of the date and time
        // however we still need to deal with the wild variety of delimiters that can be used
        //                                                                                          
        // Some strings may have a space instead of a T between date and time for example
        // Similarly some non-compliant strings may have a colon instead of a dash for the date or a slash
        // That is Ok, we treat it as a set of numbers separated by delimiters whatever those may be, 
        // trying to parse as much as possible beyond strict ISO8601 due to pragmatically the need to deal with non-compliant strings

        GUCEF::vector< Int32 > dtParts;
        GUCEF::vector< char > dtDelims;
        dtParts.reserve( 8 );
        dtDelims.reserve( 8 );
    
        char lastDelim = '0';
        bool inDigitSection = false;
        UInt32 digitSegmentStart = 0;
        bool firstDelimIsPlusMinusPrefix = false;
        bool hasMinusPrefix = false;
        bool firstNumberHadMoreThanTwoDigits = false;

        UInt32 bytesScanned=0;
        for ( ; bytesScanned < sourceBufferSize; ++bytesScanned )
        {
            char ch = *( dtBuffer + bytesScanned );
            
            if ( ch == '\0' )
            {
                if ( inDigitSection )
                {
                    Int32 number = 0;
                    int readParts = sscanf_s( dtBuffer+digitSegmentStart, "%d", &number );
                    if ( 1 == readParts )
                        dtParts.push_back( number );
                }
                break;
            }
            else
            if ( IsDigit( ch ) )
            {
                if ( !inDigitSection )
                {
                    inDigitSection = true;
                    digitSegmentStart = bytesScanned;
                }
            }
            else
            {                
                if ( lastDelim != ch )
                {
                    lastDelim = ch;
                    if ( 'Z' != ch && 'z' != ch )
                        dtDelims.push_back( ch );

                    if ( bytesScanned == 0 && ( '-' == ch || '+' == ch ) )
                    {
                        firstDelimIsPlusMinusPrefix = true;
                        hasMinusPrefix = ( '-' == ch );
                    }
                }
                if ( inDigitSection )
                {
                    inDigitSection = false;                
                    Int32 number = 0;
                    int readParts = sscanf_s( dtBuffer+digitSegmentStart, "%d", &number );
                    if ( 1 == readParts )
                        dtParts.push_back( number );

                    if ( digitSegmentStart <= 1 )
                    {
                        firstNumberHadMoreThanTwoDigits = bytesScanned-digitSegmentStart > 2;
                    }
                }            
            }
        }

        if ( dtDelims.empty() || dtParts.empty() )
            return -1; // This should never happen if we came into this section

        bool hasExplicitTzOffset = false;
        bool isNegativeTzOffset = false;
        if ( dtDelims.size() >= 5 )
        {
            // YYYY-MM-DDTHH:MM:SS.sss+HH:MM
            // ie - T : . + : 
            char tzDelim = dtDelims[ dtDelims.size()-2 ];
            if ( tzDelim == '+' )
            {
                hasExplicitTzOffset = true;
            }
            else
            if ( tzDelim == '-' )
            {
                hasExplicitTzOffset = true;
                isNegativeTzOffset = true;
            }
        }
        
        bool hasMilliseconds = false;
        if ( dtDelims.size() >= 3 )
        {
            if ( hasExplicitTzOffset )
            {
                // YYYY-MM-DDTHH:MM:SS.sss+HH:MM
                // ie - T : . + : 
                char msDelim = dtDelims[ dtDelims.size()-3 ];
                hasMilliseconds = msDelim == '.';
            }
            else
            {
                // YYYY-MM-DDTHH:MM:SS.sss
                // ie - T : .
                hasMilliseconds = dtDelims.back() == '.';
            }
        }
        
        bool hasDate = dtParts.size() >= 6 || 
                      ( dtParts.size() < 6 && dtParts.size() >= 2 && firstNumberHadMoreThanTwoDigits );

        bool hasTime = dtParts.size() >= 6 || 
                       ( dtParts.size() < 6 && dtParts.size() >= 3 && !firstNumberHadMoreThanTwoDigits );

        if ( hasDate && hasTime && hasMilliseconds && hasExplicitTzOffset && dtParts.size() >= 7 )
        {
            // YYYY-MM-DDTHH:MM:SS.sss+HH:MM
            TimezoneOffsetInMinsFromIso8601DateTimeStringParts( dtParts, isNegativeTzOffset, tzOffset );
            Set( (Int16) dtParts[0], (UInt8) dtParts[1], (UInt8) dtParts[2], 
                 (Int8) dtParts[3], (Int8) dtParts[4], (Int8) dtParts[5], (Int16) dtParts[6], tzOffset );
        }
        else
        if ( hasDate && hasTime && hasMilliseconds && !hasExplicitTzOffset && dtParts.size() >= 7 )
        {
            // YYYY-MM-DDTHH:MM:SS.sss
            Set( (Int16) dtParts[0], (UInt8) dtParts[1], (UInt8) dtParts[2], 
                 (Int8) dtParts[3], (Int8) dtParts[4], (Int8) dtParts[5], (Int16) dtParts[6] );
        }
        else
        if ( hasDate && hasTime && !hasMilliseconds && hasExplicitTzOffset && dtParts.size() >= 6 )
        {
            // YYYY-MM-DDTHH:MM:SS+HH:MM
            TimezoneOffsetInMinsFromIso8601DateTimeStringParts( dtParts, isNegativeTzOffset, tzOffset );
            Set( (Int16) dtParts[0], (UInt8) dtParts[1], (UInt8) dtParts[2], 
                 (Int8) dtParts[3], (Int8) dtParts[4], (Int8) dtParts[5], (Int16) 0, tzOffset );
        }
        else
        if ( hasDate && hasTime && !hasMilliseconds && !hasExplicitTzOffset && dtParts.size() >= 6 )
        {
            // YYYY-MM-DDTHH:MM:SS
            Set( (Int16) dtParts[0], (UInt8) dtParts[1], (UInt8) dtParts[2], 
                 (Int8) dtParts[3], (Int8) dtParts[4], (Int8) dtParts[5] );
        }
        else
        if ( hasDate && !hasTime && !hasExplicitTzOffset && dtParts.size() >= 3 )
        {
            // YYYY-MM-DD
            Set( (Int16) dtParts[0], (UInt8) dtParts[1], (UInt8) dtParts[2] );
        }
        else
        if ( hasDate && !hasTime && !hasExplicitTzOffset && dtParts.size() == 2 )
        {
            // YYYY-DOY
            // This is an Ordinal date format
            Int32 dayOfYear = dtParts[1]; // note that this is 1 based, not 0 based
            Set( (Int16) dtParts[0], (UInt8) 1, (UInt8) 1 );
            AddDays( dayOfYear - 1 );
        }
        else
        if ( !hasDate && hasTime && !hasMilliseconds && !hasExplicitTzOffset && dtParts.size() >= 3 )
        {
            // HH:MM:SS            
            if ( !hasMinusPrefix )
                Set( (Int16) 0, (UInt8) 0, (UInt8) 0, 
                     (Int8) dtParts[0], (Int8) dtParts[1], (Int8) dtParts[2] );
            else
                Set( (Int16) 0, (UInt8) 0, (UInt8) 0, 
                     (Int8) -dtParts[0], (Int8) -dtParts[1], (Int8) -dtParts[2] );
        }
        else
        if ( !hasDate && hasTime && hasMilliseconds && !hasExplicitTzOffset && dtParts.size() >= 4 )
        {
            // HH:MM:SS.sss
            if ( !hasMinusPrefix )
                Set( (Int16) 0, (UInt8) 0, (UInt8) 0, 
                     (Int8) dtParts[0], (Int8) dtParts[1], (Int8) dtParts[2], (Int16) dtParts[3] );
            else
                Set( (Int16) 0, (UInt8) 0, (UInt8) 0, 
                     (Int8) -dtParts[0], (Int8) -dtParts[1], (Int8) -dtParts[2], (Int16) -dtParts[3] );
        }

        return bytesScanned;
    }
    else
    {
        // This is the case where we have a string with no delimiters
        // In such a case we need to determine the format based on the length of the string
        // since each component has a fixed length (unless the string is invalid)
        Int32 year=0, month=0, day=0, hours=0, minutes=0, seconds=0, milliseconds=0;        

        const char* nullTermPtr = (const char*) memchr( dtBuffer, '\0', sourceBufferSize );
        UInt32 dataLength = GUCEF_NULL != nullTermPtr ? (UInt32)(nullTermPtr-dtBuffer) : sourceBufferSize;

        if ( dataLength < 6 )
            return -1;
        
        const char* milliSecDelimPtr = (const char*) memchr( dtBuffer+6, '.', dataLength-6 );
        if ( GUCEF_NULL == milliSecDelimPtr )
            milliSecDelimPtr = (const char*) memchr( dtBuffer+6, ',', dataLength-6 );
        bool hasMilliseconds = GUCEF_NULL != milliSecDelimPtr;

        Int32 primarySectionLength = hasMilliseconds ? (Int32)(milliSecDelimPtr-dtBuffer) : (Int32)dataLength;
        
        // first parse the primary section
        
        if ( primarySectionLength == 6 ) 
        {
            // Non-standard and somehwat ambigious format, best effort parsing
            // As a general rule regardless:
            //      "The ISO 8601 standard does not allow the use of two-digit years, 
            //       as this format was removed in ISO 8601:2004. 
            //       The standard defines the year format as YYYY, which is a four-digit year"
            //
            // HHMMSS
            // Length = time(2+2+2) = 6 chars, 3 total parts of which 3 to parse
            int readParts = sscanf_s( dtBuffer, "%02d%02d%02d", &hours, &minutes, &seconds );
            if ( readParts != 3 )
                return -1;
        }
        else
        if ( primarySectionLength == 7 && 'T' == dtBuffer[ 0 ] ) 
        {
            // THHMMSS
            // Length = date(4+2+2)+T+time(2+2+2) = 15 chars, 7 total parts of which 6 to parse
            int readParts = sscanf_s( dtBuffer+1, "%02d%02d%02d", &hours, &minutes, &seconds );
            if ( readParts != 3 )
                return -1;
        }
        else
        if ( primarySectionLength == 8 ) 
        {
            // YYYYMMDD
            // Length = date(4+2+2) = 8 chars, 3 total parts of which 3 to parse
            int readParts = sscanf_s( dtBuffer, "%04d%02d%02d", &year, &month, &day );
            if ( readParts != 3 )
                return -1;
        }
        else
        if ( primarySectionLength == 15 && 'T' == dtBuffer[ 8 ] ) 
        {
            // YYYYMMDDTHHMMSS
            // Length = date(4+2+2)+T+time(2+2+2) = 15 chars, 7 total parts of which 6 to parse
            int readParts = sscanf_s( dtBuffer, "%04d%02u%02u%*c%02d%02d%02d", &year, &month, &day, &hours, &minutes, &seconds );
            if ( readParts != 6 )
                return -1;
        }
        else
        if ( primarySectionLength == 14 ) // YYYYMMDDHHMMSS
        {
            // YYYYMMDDHHMMSS
            // Length = date(4+2+2)+time(2+2+2) = 14 chars, 6 total parts of which 6 to parse
            int readParts = sscanf_s( dtBuffer, "%04d%02u%02u%02d%02d%02d", &year, &month, &day, &hours, &minutes, &seconds );
            if ( readParts != 6 )
                return -1;
        }
        
        // Now parse the milliseconds if they exist
        
        if ( hasMilliseconds && primarySectionLength+3 < (Int32) dataLength )
        {
            int readParts = sscanf_s( milliSecDelimPtr+1, "%03d", &milliseconds );
            if ( readParts != 1 )
                return -1;
            
            primarySectionLength += 4; // dot plus 3 digits
        }

        // Followed by the timezone offset if it exists
        Int32 tzBytes = TimezoneOffsetInMinsFromIso8601DateTimeStringRemnant( dtBuffer+primarySectionLength, dataLength-primarySectionLength, tzOffset );

        // Now actually set the values we parsed
        Set( (Int16) year, (UInt8) month, (UInt8) day, (Int8) hours, (Int8) minutes, (Int8) seconds, (Int16) milliseconds, tzOffset );

        if ( tzBytes > 0 )
            return primarySectionLength + tzBytes;
        return primarySectionLength;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CDateTime::FromIso8601TimeString( const CString& source )
{GUCEF_TRACE;

    // Our DT implementation can also handle time only strings
    // Simply redirect there. Its done that way for robustness in case a string is not what was expected 
    // yet you can still express via member function names what the expected content was
    return 0 < FromIso8601DateTimeString( source.C_String(), source.ByteSize() );
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::FromIso8601DateTimeString( const CDynamicBuffer& source, UInt32 sourceBufferOffset )
{GUCEF_TRACE;

    return FromIso8601DateTimeString( source.GetConstBufferPtr( sourceBufferOffset ), source.GetDataSize() - sourceBufferOffset );
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::FromIso8601DateTimeString( const CString& source )
{GUCEF_TRACE;

    return 0 < FromIso8601DateTimeString( source.C_String(), source.ByteSize() );
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::ToIso8601DateTimeString( void* targetBuffer, UInt32 targetBufferSize, bool includeDelimeters, bool includeMilliseconds ) const
{GUCEF_TRACE;

    if ( !IsUTC() )
    {
        return ToUTC().ToIso8601DateTimeString( targetBuffer, targetBufferSize, includeDelimeters, includeMilliseconds );
    }

    char* dtBuffer = static_cast< char* >( targetBuffer );

    // The ISO DateTime range is likely different from the host O/S range
    // As such we should always clamp to the ISO supported range
    Int16 isoClampedYear = m_year;
    if ( isoClampedYear > 9999 )
        isoClampedYear = 9999;
    else
    if ( isoClampedYear < 0000 )
        isoClampedYear = 0000;

    if ( includeMilliseconds )
    {
        // ISO 8601 requires the miliseconds to be expressed as a fraction of a second
        // a second has 1000 milliseconds thus we use 3 digits to denote the fraction at which point it directly translates as said fraction
        // Note that ISO8601:2004 allows: "as many digits as necessary following the decimal sign. A decimal fraction shall have at least one digit"
        UInt16 milliSecs = m_milliseconds;
        if ( milliSecs > 999 )
            milliSecs = 0;

        if ( includeDelimeters )
        {
            // Length = date(4+1+2+1+2)+1+time(2+1+2+1+2+1+3) = 24 chars
            return sprintf_s( dtBuffer, targetBufferSize, "%04d-%02u-%02uT%02u:%02u:%02u.%03uZ", isoClampedYear, m_month, m_day, m_hours, m_minutes, m_seconds, m_milliseconds );
        }
        else
        {
            // Length = date(4+2+2)+time(2+2+2+1+3) = 18 chars
            return sprintf_s( dtBuffer, targetBufferSize, "%04d%02u%02u%02u%02u%02u.%03uZ", isoClampedYear, m_month, m_day, m_hours, m_minutes, m_seconds, m_milliseconds );
        }
    }
    else
    {
        if ( includeDelimeters )
        {
            // Length = date(4+1+2+1+2)+1+time(2+1+2+1+2) = 20 chars
            return sprintf_s( dtBuffer, targetBufferSize, "%04d-%02u-%02uT%02u:%02u:%02uZ", isoClampedYear, m_month, m_day, m_hours, m_minutes, m_seconds );
        }
        else
        {
            // Length = date(4+2+2)+time(2+2+2) = 14 chars
            return sprintf_s( dtBuffer, targetBufferSize, "%04d%02u%02u%02u%02u%02u", isoClampedYear, m_month, m_day, m_hours, m_minutes, m_seconds );
        }
    }
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::ToIso8601DateString( void* targetBuffer, UInt32 targetBufferSize, bool includeDelimeters ) const
{GUCEF_TRACE;

    if ( !IsUTC() )
    {
        return ToUTC().ToIso8601DateString( targetBuffer, targetBufferSize, includeDelimeters );
    }

    char* dtBuffer = static_cast< char* >( targetBuffer );

    // The ISO DateTime range is likely different from the host O/S range
    // As such we should always clamp to the ISO supported range
    Int16 isoClampedYear = m_year;
    if ( isoClampedYear > 9999 )
        isoClampedYear = 9999;
    else
    if ( isoClampedYear < 0000 )
        isoClampedYear = 0000;

    if ( includeDelimeters )
    {
        // Length = date(4+1+2+1+2) = 9 chars
        return sprintf_s( dtBuffer, targetBufferSize, "%04d-%02u-%02u", isoClampedYear, m_month, m_day );
    }
    else
    {
        // Length = date(4+2+2) = 8 chars
        return sprintf_s( dtBuffer, targetBufferSize, "%04d%02u%02u", isoClampedYear, m_month, m_day );
    }
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::ToIso8601TimeString( void* targetBuffer, UInt32 targetBufferSize, bool includeDelimeters ) const
{GUCEF_TRACE;

    char* dtBuffer = static_cast< char* >( targetBuffer );

    bool isNegativeTime = m_hours < 0 || m_minutes < 0 || m_seconds < 0 || m_milliseconds < 0;
    char sign = isNegativeTime ? '-' : '+';
    Int32 hours = isNegativeTime && m_hours < 0 ? -m_hours : m_hours;
    Int32 minutes = isNegativeTime && m_minutes < 0 ? -m_minutes : m_minutes;
    Int32 seconds = isNegativeTime && m_seconds < 0 ? -m_seconds : m_seconds;
    Int32 milliseconds = isNegativeTime && m_milliseconds < 0 ? -m_milliseconds : m_milliseconds;

    if ( includeDelimeters )
    {
        // +HH:MM:SS.sss
        // Length = time(1+2+1+2+1+2+1+3) = 13 chars
        return sprintf_s( dtBuffer, targetBufferSize, "%c%02i:%02i:%02i.%03i", sign, hours, minutes, seconds, milliseconds );
    }
    else
    {
        // +HHMMSSsss
        // Length = time(1+2+2+2+1+3) = 11 chars
        return sprintf_s( dtBuffer, targetBufferSize, "%c%02i%02i%02i.%03i", sign, hours, minutes, seconds, milliseconds );
    }
}

/*-------------------------------------------------------------------------*/

Int32
CDateTime::ToIso8601DateTimeString( CDynamicBuffer& target, UInt32 targetBufferOffset, bool includeDelimeters, bool includeMilliseconds ) const
{GUCEF_TRACE;

    if ( target.GetAutoEnlarge() && target.GetRemainingBufferSize( targetBufferOffset ) < 25 )
    {
        if ( !target.SetBufferSize( target.GetBufferSize() + 25 ) )
            return -1;
    }

    return ToIso8601DateTimeString( target.GetBufferPtr( targetBufferOffset )           ,
                                    target.GetRemainingBufferSize( targetBufferOffset ) ,
                                    includeDelimeters                                   ,
                                    includeMilliseconds                                 );
}

/*-------------------------------------------------------------------------*/

CString
CDateTime::ToIso8601DateTimeString( bool includeDelimeters, bool includeMilliseconds ) const
{GUCEF_TRACE;

    // We cannot just assume our data structure holds values in a specific range
    // The data storage in the class specifically is more tolerant
    //
    // Int16 year <- thus "-32768" thus 6 chars
    // UInt8 month <- thus "256" thus 3 chars
    // UInt8 day <- thus "256" thus 3 chars
    // UInt8 hours <- thus "256" thus 3 chars
    // UInt8 minutes <- thus "256" thus 3 chars
    // UInt8 seconds <- thus "256" thus 3 chars
    // UInt16 milliseconds <- this is reinterpreted and clamped to a precision of 0.xxx fraction thus 3 chars
    // --------------------------------------  +
    // Thus with assuming garbage input is a possibility we need space for 24 chars for just the digits
    // Adding in the seperators for the ISO 8601 we get +7 chars
    // This gives us a total max with garbage input of 31 chars plus null terminator thus 32 chars

    char dtBuffer[ 32 ];
    if ( 0 < ToIso8601DateTimeString( dtBuffer, sizeof(dtBuffer), includeDelimeters, includeMilliseconds ) )
        return dtBuffer;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString 
CDateTime::ToIso8601DateString( bool includeDelimeters ) const
{GUCEF_TRACE;

    // We cannot just assume our data structure holds values in a specific range
    // The data storage in the class specifically is more tolerant
    //
    // Int16 year <- thus "-32768" thus 6 chars
    // UInt8 month <- thus "256" thus 3 chars
    // UInt8 day <- thus "256" thus 3 chars
    // --------------------------------------  +
    // Thus with assuming garbage input is a possibility we need space for 12 chars for just the digits
    // Adding in the seperators for the ISO 8601 we get +3 chars
    // This gives us a total max with garbage input of 15 chars plus null terminator thus 16 chars

    char dtBuffer[ 16 ];
    if ( 0 < ToIso8601DateString( dtBuffer, sizeof(dtBuffer), includeDelimeters ) )
        return dtBuffer;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString 
CDateTime::ToIso8601TimeString( bool includeDelimeters ) const
{GUCEF_TRACE;

    // We cannot just assume our data structure holds values in a specific range
    // The data storage in the class specifically is more tolerant
    //
    // Int16 hours <- thus "-32768" thus 6 chars
    // Int8 minutes <- thus "-128" thus 4 chars
    // Int8 seconds <- thus "-128" thus 4 chars
    // Int16 milliseconds <- thus "-32768" thus 6 chars
    // --------------------------------------  +
    // Thus with assuming garbage input is a possibility we need space for 20 chars for just the digits
    // Adding in the seperators for the ISO 8601 we get +3 chars
    // This gives us a total max with garbage input of 23 chars plus null terminator thus 24 chars

    char dtBuffer[ 24 ];
    if ( 0 < ToIso8601TimeString( dtBuffer, sizeof(dtBuffer), includeDelimeters ) )
        return dtBuffer;
    return CString::Empty;
}
/*-------------------------------------------------------------------------*/

void
CDateTime::FromTmStruct( const struct tm* tmStruct, bool isUtc )
{GUCEF_TRACE;

    Set();
    if ( GUCEF_NULL == tmStruct )
        return;

    m_year = (Int16) tmStruct->tm_year + 1900; // years since 1900 for tm struct
    m_month = (UInt8) tmStruct->tm_mon + 1;    // zero based for tm struct
    m_day = (UInt8) tmStruct->tm_mday;
    m_hours = (UInt8) tmStruct->tm_hour;
    m_minutes = (UInt8) tmStruct->tm_min;
    m_seconds = (UInt8) tmStruct->tm_sec;

    // We dont currently take DST into account
    if ( !isUtc )
        m_timezoneOffsetInMins = GetTimeZoneUTCOffsetInMins();
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::IsWithinRange( const CDateTime& a, const CDateTime& b, bool includingBoundaryItself ) const
{GUCEF_TRACE;

    const CDateTime* minDt = GUCEF_NULL;
    const CDateTime* maxDt = GUCEF_NULL;

    if ( a < b )
    {
        minDt = &a;
        maxDt = &b;
    }
    else
    {
        minDt = &b;
        maxDt = &a;
    }

    if ( includingBoundaryItself )
    {
        return (*this) >= (*minDt) && (*this) <= (*maxDt);
    }
    else
    {
        return (*this) > (*minDt) && (*this) < (*maxDt);
    }
}

/*-------------------------------------------------------------------------*/

void
CDateTime::Clear( void )
{GUCEF_TRACE;

    *this = Empty;
}

/*-------------------------------------------------------------------------*/

void
CDateTime::AddSeconds( Int32 secondsToAdd )
{GUCEF_TRACE;

    UInt64 baseTimestamp = ToUnixEpochBasedTicksInMillisecs();
    Int64 msDelta = (Int64) secondsToAdd * 1000;
    baseTimestamp += msDelta;
    FromUnixEpochBasedTicksInMillisecs( baseTimestamp );    
}

/*-------------------------------------------------------------------------*/

void
CDateTime::AddMinutes( Int32 minutesToAdd )
{GUCEF_TRACE;

    UInt64 baseTimestamp = ToUnixEpochBasedTicksInMillisecs();
    Int64 msDelta = (Int64) minutesToAdd * 60 * 1000;
    baseTimestamp += msDelta;
    FromUnixEpochBasedTicksInMillisecs( baseTimestamp );    
}

/*-------------------------------------------------------------------------*/

void
CDateTime::AddHours( Int32 hoursToAdd )
{GUCEF_TRACE;

    UInt64 baseTimestamp = ToUnixEpochBasedTicksInMillisecs();
    Int64 msDelta = (Int64) hoursToAdd * 60 * 60 * 1000;
    baseTimestamp += msDelta;
    FromUnixEpochBasedTicksInMillisecs( baseTimestamp );    
}

/*-------------------------------------------------------------------------*/

void 
CDateTime::AddTime( const CTime& timeToAdd )
{GUCEF_TRACE;

    UInt64 baseTimestamp = ToUnixEpochBasedTicksInMillisecs();
    Int64 msDelta =  (Int64) timeToAdd.GetHours() * 60 * 60 * 1000;
          msDelta += (Int64) timeToAdd.GetMinutes() * 60 * 1000;
          msDelta += (Int64) timeToAdd.GetSeconds() * 1000;
          msDelta += (Int64) timeToAdd.GetMilliseconds();
    baseTimestamp += msDelta;
    FromUnixEpochBasedTicksInMillisecs( baseTimestamp );    
}

/*-------------------------------------------------------------------------*/

void 
CDateTime::AddDays( Int32 daysToAdd )
{GUCEF_TRACE;

    UInt64 baseTimestamp = ToUnixEpochBasedTicksInMillisecs();
    Int64 msDelta = (Int64) daysToAdd * 24 * 60 * 60 * 1000;
    baseTimestamp += msDelta;
    FromUnixEpochBasedTicksInMillisecs( baseTimestamp );    
}

/*-------------------------------------------------------------------------*/

void 
CDateTime::AddWeeks( Int32 weeksToAdd )
{GUCEF_TRACE;

    UInt64 baseTimestamp = ToUnixEpochBasedTicksInMillisecs();
    Int64 msDelta = (Int64) weeksToAdd * 7 * 24 * 60 * 60 * 1000;
    baseTimestamp += msDelta;
    FromUnixEpochBasedTicksInMillisecs( baseTimestamp );    
}

/*-------------------------------------------------------------------------*/

CDateTime::TDateTimeComponent 
CDateTime::ToDateTimeComponentType( const CORE::CString& dateTimeComponentType )
{GUCEF_TRACE;

    CORE::CString dateTimeComponentTypeLc = dateTimeComponentType.Lowercase();
    
    if ( "year" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_YEAR;
    if ( "month" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_MONTH;
    if ( "day" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_DAY;
    if ( "hour" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_HOURS;
    if ( "minute" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_MINUTES;
    if ( "second" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_SECONDS;
    if ( "millisecond" == dateTimeComponentTypeLc )
        return TDateTimeComponent::DATETIMECOMPONENT_MILLISECONDS;

    return TDateTimeComponent::DATETIMECOMPONENT_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
CDateTime::ClampToBoundary( TDateTimeComponent boundary , 
                            TBoundaryType boundaryType  , 
                            CORE::CDateTime& clampedDt  ) const
{GUCEF_TRACE;

    clampedDt = *this;

    switch ( boundary )
    {
        case TDateTimeComponent::DATETIMECOMPONENT_YEAR:
        {
            if ( TBoundaryType::BOUNDARYTYPE_MINIMUM == boundaryType )
            {
                clampedDt.SetMonth( 1 );
                clampedDt.SetDay( 1 );
                clampedDt.SetHours( 0 );
                clampedDt.SetMinutes( 0 );
                clampedDt.SetSeconds( 0 );
                clampedDt.SetMilliseconds( 0 );
            }
            else
            if ( TBoundaryType::BOUNDARYTYPE_MAXIMUM == boundaryType )
            {
                clampedDt.SetMonth( 12 );
                clampedDt.SetDay( GetDaysInMonth() );
                clampedDt.SetHours( 23 );
                clampedDt.SetMinutes( 59 );
                clampedDt.SetSeconds( 59 );
                clampedDt.SetMilliseconds( 999 );
            }
            return true;
        }
        case TDateTimeComponent::DATETIMECOMPONENT_MONTH:
        {
            if ( TBoundaryType::BOUNDARYTYPE_MINIMUM == boundaryType )
            {
                clampedDt.SetDay( 1 );
                clampedDt.SetHours( 0 );
                clampedDt.SetMinutes( 0 );
                clampedDt.SetSeconds( 0 );
                clampedDt.SetMilliseconds( 0 );
            }
            else
            if ( TBoundaryType::BOUNDARYTYPE_MAXIMUM == boundaryType )
            {
                clampedDt.SetDay( GetDaysInMonth() );
                clampedDt.SetHours( 23 );
                clampedDt.SetMinutes( 59 );
                clampedDt.SetSeconds( 59 );
                clampedDt.SetMilliseconds( 999 );
            }
            return true;
        }
        case TDateTimeComponent::DATETIMECOMPONENT_DAY:
        {
            if ( TBoundaryType::BOUNDARYTYPE_MINIMUM == boundaryType )
            {
                clampedDt.SetHours( 0 );
                clampedDt.SetMinutes( 0 );
                clampedDt.SetSeconds( 0 );
                clampedDt.SetMilliseconds( 0 );
            }
            else
            if ( TBoundaryType::BOUNDARYTYPE_MAXIMUM == boundaryType )
            {
                clampedDt.SetHours( 23 );
                clampedDt.SetMinutes( 59 );
                clampedDt.SetSeconds( 59 );
                clampedDt.SetMilliseconds( 999 );
            }
            return true;
        }
        case TDateTimeComponent::DATETIMECOMPONENT_HOURS:
        {
            if ( TBoundaryType::BOUNDARYTYPE_MINIMUM == boundaryType )
            {
                clampedDt.SetMinutes( 0 );
                clampedDt.SetSeconds( 0 );
                clampedDt.SetMilliseconds( 0 );
            }
            else
            if ( TBoundaryType::BOUNDARYTYPE_MAXIMUM == boundaryType )
            {
                clampedDt.SetMinutes( 59 );
                clampedDt.SetSeconds( 59 );
                clampedDt.SetMilliseconds( 999 );
            }
            return true;
        }
        case TDateTimeComponent::DATETIMECOMPONENT_MINUTES:
        {
            if ( TBoundaryType::BOUNDARYTYPE_MINIMUM == boundaryType )
            {
                clampedDt.SetSeconds( 0 );
                clampedDt.SetMilliseconds( 0 );
            }
            else
            if ( TBoundaryType::BOUNDARYTYPE_MAXIMUM == boundaryType )
            {
                clampedDt.SetSeconds( 59 );
                clampedDt.SetMilliseconds( 999 );
            }
            return true;
        }
        case TDateTimeComponent::DATETIMECOMPONENT_SECONDS:
        {
            if ( TBoundaryType::BOUNDARYTYPE_MINIMUM == boundaryType )
            {
                clampedDt.SetMilliseconds( 0 );
            }
            else
            if ( TBoundaryType::BOUNDARYTYPE_MAXIMUM == boundaryType )
            {
                clampedDt.SetMilliseconds( 999 );
            }
            return true;
        }
        case TDateTimeComponent::DATETIMECOMPONENT_MILLISECONDS:
        {
            // no-op clamp: full precision
            return true;
        }
        
        case TDateTimeComponent::DATETIMECOMPONENT_TIMEZONEOFFSETINMINS:
        case TDateTimeComponent::DATETIMECOMPONENT_UNKNOWN:
        default:
        {
            // non-sensical in this context
            return false;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
