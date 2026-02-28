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
#include <time.h>

#include "gucefCORE_CDate.h"

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #include <Windows.h>

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

CDate::CDate( void )
    : CIDate()
    , m_year( 0 )
    , m_month( 0 )
    , m_day( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDate::CDate( const time_t& src, bool isUtc )
    : CIDate()
    , m_year( 0 )
    , m_month( 0 )
    , m_day( 0 )
{GUCEF_TRACE;

    struct tm* brokenDownTime = GUCEF_NULL;
    if ( isUtc )
        brokenDownTime = gmtime( &src );
    else
        brokenDownTime = localtime( &src );

    m_year = (Int16) brokenDownTime->tm_year+1900;
    m_month = (UInt8) brokenDownTime->tm_mon+1;
    m_day = (UInt8) brokenDownTime->tm_wday+1;
}

/*-------------------------------------------------------------------------*/

CDate::CDate( const CDate& src )
    : CIDate( src )
    , m_year( src.m_year )
    , m_month( src.m_month )
    , m_day( src.m_day )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDate::CDate( UInt16 year, UInt8 month, UInt8 day )
    : CIDate()
    , m_year( year )
    , m_month( month )
    , m_day( day )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDate::~CDate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDate
CDate::TodaysUTCDate( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::SYSTEMTIME systemTime;
    ::GetSystemTime( &systemTime );
    return CDate( (UInt16) systemTime.wYear, (UInt8) systemTime.wMonth, (UInt8) systemTime.wDay );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    struct timespec time;
    ::memset( &time, 0, sizeof(time) );
    ::clock_gettime( CLOCK_REALTIME, &time );
    struct tm utcTime;
    ::gmtime_r( &time.tv_sec, &utcTime );
    return CDate( (Int16) utcTime.tm_year+1900,
                  (UInt8) utcTime.tm_mon+1,
                  (UInt8) utcTime.tm_wday+1);

    #else

    // Not supported for the current platform
    return CDate( 0, 0, 0 );
    #endif
}

/*-------------------------------------------------------------------------*/

CDate
CDate::TodaysLocalDate( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::SYSTEMTIME systemTime;
    ::GetLocalTime( &systemTime );
    return CDate( (UInt16) systemTime.wYear, (UInt8) systemTime.wMonth, (UInt8) systemTime.wDay );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    struct timespec time;
    ::memset( &time, 0, sizeof(time) );
    ::clock_gettime( CLOCK_REALTIME, &time );
    struct tm hereTime;
    ::localtime_r( &time.tv_sec, &hereTime );
    return CDate( (Int16) hereTime.tm_year+1900,
                  (UInt8) hereTime.tm_mon+1,
                  (UInt8) hereTime.tm_wday+1);

    #else

    // Not supported for the current platform
    return CDate( 0, 0, 0 );
    #endif
}

/*-------------------------------------------------------------------------*/

CDate
CDate::UnixEpochDate( void )
{GUCEF_TRACE;

    return CDate( 1970, 1, 1 );
}

/*-------------------------------------------------------------------------*/

bool 
CDate::SetYear( Int16 year )
{GUCEF_TRACE;

    m_year = year;
    return true;
}

/*-------------------------------------------------------------------------*/

Int16
CDate::GetYear( void ) const
{GUCEF_TRACE;

    return m_year;
}

/*-------------------------------------------------------------------------*/

bool 
CDate::SetMonth( UInt8 month )
{GUCEF_TRACE;

    m_month = month;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt8
CDate::GetMonth( void ) const
{GUCEF_TRACE;

    return m_month;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDate::GetMonthName( void ) const
{GUCEF_TRACE;

    static const char* monthNames[] = { "January", "February", "March", "April", "May", "June",
                                        "July", "August", "September", "October", "November", "December" };
    if ( m_month >= 1 && m_month <= 12 )
        return monthNames[ m_month-1 ];
    else
        return "Unknown";
}

/*-------------------------------------------------------------------------*/

bool 
CDate::SetDay( UInt8 day )
{GUCEF_TRACE;

    m_day = day;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt8
CDate::GetDay( void ) const
{GUCEF_TRACE;

    return m_day;
}

/*-------------------------------------------------------------------------*/

CDate&
CDate::operator=( const CDate& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_year = src.m_year;
        m_month = src.m_month;
        m_day = src.m_day;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDate::operator==( const CDate& src ) const
{GUCEF_TRACE;

    return m_year == src.m_year && m_month == src.m_month && m_day == src.m_day;
}

/*-------------------------------------------------------------------------*/

bool
CDate::operator!=( const CDate& src ) const
{GUCEF_TRACE;

    return m_year != src.m_year || m_month != src.m_month || m_day != src.m_day;
}

/*-------------------------------------------------------------------------*/

CString 
CDate::ToIso8601DateString( bool includeDelimeters ) const
{GUCEF_TRACE;

    return CDateTime( *this, true ).ToIso8601DateString( includeDelimeters );
}

/*-------------------------------------------------------------------------*/

bool 
CDate::IsLeapYear( void ) const
{GUCEF_TRACE;

    // leap year if perfectly divisible by 400
    if ( m_year % 400 == 0 ) 
        return true;

    // not a leap year if divisible by 100
    // but not divisible by 400
    if ( m_year % 100 == 0 ) 
        return false;

    // leap year if not divisible by 100
    // but divisible by 4
    if ( m_year % 4 == 0 ) 
        return true;

    // all other years are not leap years
    return false;
}

/*-------------------------------------------------------------------------*/

UInt8 
CDate::GetDaysInMonth( void ) const
{GUCEF_TRACE;

    static const UInt8 daysInMonth[] = { 31, 28, 31, 30, 31, 30,
                                         31, 31, 30, 31, 30, 31 };
    if ( m_month >= 1 && m_month <= 12 )
    {
        if ( m_month == 2 && IsLeapYear() )
            return 29;
        else
            return daysInMonth[ m_month-1 ];
    }
    else
        return 0;
}

/*-------------------------------------------------------------------------*/

UInt8
CDate::GetDayOfWeek( void ) const
{GUCEF_TRACE;

    // Tomohiko Sakamoto's algorithm. Returns 0=Sunday, 1=Monday, ..., 6=Saturday
    static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    int y = (int) m_year;
    int m = (int) m_month;
    int d = (int) m_day;
    if ( m < 3 ) --y;
    return (UInt8) ( ( y + y/4 - y/100 + y/400 + t[m-1] + d ) % 7 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
