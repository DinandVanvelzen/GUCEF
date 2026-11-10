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

#include "gucefCORE_CTime.h"

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

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    #include <sys/time.h>

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

CTime::CTime( void )
    : CITime()
    , m_hours( 0 )
    , m_minutes( 0 )
    , m_seconds( 0 )
    , m_milliseconds( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTime::CTime( const time_t& src, bool isUtc )
    : CITime()
    , m_hours( 0 )
    , m_minutes( 0 )
    , m_seconds( 0 )
    , m_milliseconds( 0 )
{GUCEF_TRACE;

    struct tm* brokenDownTime = GUCEF_NULL;
    if ( isUtc )
        brokenDownTime = gmtime( &src );
    else
        brokenDownTime = localtime( &src );

    m_hours = (Int8) brokenDownTime->tm_hour;
    m_minutes = (Int8) brokenDownTime->tm_min;
    m_seconds = (Int8) brokenDownTime->tm_sec;
}

/*-------------------------------------------------------------------------*/

CTime::CTime( const CTime& src )
    : CITime( src )
    , m_hours( src.m_hours )
    , m_minutes( src.m_minutes )
    , m_seconds( src.m_seconds )
    , m_milliseconds( src.m_milliseconds )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTime::CTime( Int8 hours, Int8 minutes, Int8 seconds, Int16 milliseconds )
    : CITime()
    , m_hours( hours )
    , m_minutes( minutes )
    , m_seconds( seconds )
    , m_milliseconds( milliseconds )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTime::~CTime()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTime
CTime::NowUTCTime( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::SYSTEMTIME systemTime;
    ::GetSystemTime( &systemTime );
    return CTime( (Int8) systemTime.wHour, (Int8) systemTime.wMinute, (Int8) systemTime.wSecond, (Int16) systemTime.wMilliseconds );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    struct timespec time;
    ::memset( &time, 0, sizeof(time) );
    ::clock_gettime( CLOCK_REALTIME, &time );
    struct tm* utcTime = ::gmtime( &time.tv_sec );
    return CTime( (Int8) utcTime->tm_hour, (Int8) utcTime->tm_min, (Int8) utcTime->tm_sec, (Int16) ( time.tv_nsec / 1000000 ) );

    #else

    // Not supported for the current platform
    return CTime();
    #endif
}

/*-------------------------------------------------------------------------*/

CTime
CTime::NowLocalTime( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::SYSTEMTIME systemTime;
    ::GetLocalTime( &systemTime );
    return CTime( (Int8) systemTime.wHour, (Int8) systemTime.wMinute, (Int8) systemTime.wSecond, (Int8) systemTime.wMilliseconds );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    struct timespec time;
    ::memset( &time, 0, sizeof(time) );
    ::clock_gettime( CLOCK_REALTIME, &time );
    struct tm* utcTime = ::localtime( &time.tv_sec );
    return CTime( (Int8) utcTime->tm_hour, (Int8) utcTime->tm_min, (Int8) utcTime->tm_sec, (Int16) ( time.tv_nsec / 1000000 ) );

    #else

    // Not supported for the current platform
    return CTime();
    #endif
}

/*-------------------------------------------------------------------------*/

void 
CTime::SetHours( Int8 hours )
{GUCEF_TRACE;

    m_hours = hours;
}

/*-------------------------------------------------------------------------*/

Int8
CTime::GetHours( void ) const
{GUCEF_TRACE;

    return m_hours;
}

/*-------------------------------------------------------------------------*/

void 
CTime::SetMinutes( Int8 minutes )
{GUCEF_TRACE;

    m_minutes = minutes;
}

/*-------------------------------------------------------------------------*/

Int8
CTime::GetMinutes( void ) const
{GUCEF_TRACE;

    return m_minutes;
}

/*-------------------------------------------------------------------------*/

void 
CTime::SetSeconds( Int8 seconds )
{GUCEF_TRACE;

    m_seconds = seconds;
}

/*-------------------------------------------------------------------------*/

Int8
CTime::GetSeconds( void ) const
{GUCEF_TRACE;

    return m_seconds;
}

/*-------------------------------------------------------------------------*/

void 
CTime::SetMilliseconds( Int16 milliseconds )
{GUCEF_TRACE;

    m_milliseconds = milliseconds;
}

/*-------------------------------------------------------------------------*/

Int16
CTime::GetMilliseconds( void ) const
{GUCEF_TRACE;

    return m_milliseconds;
}

/*-------------------------------------------------------------------------*/

CTime&
CTime::operator=( const CTime& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_hours = src.m_hours;
        m_minutes = src.m_minutes;
        m_seconds = src.m_seconds;
        m_milliseconds = src.m_milliseconds;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CTime::operator==( const CTime& src ) const
{GUCEF_TRACE;

    return m_hours == src.m_hours && m_minutes == src.m_minutes && m_seconds == src.m_seconds && m_milliseconds == src.m_milliseconds;
}

/*-------------------------------------------------------------------------*/

bool
CTime::operator!=( const CTime& src ) const
{GUCEF_TRACE;

    return m_hours != src.m_hours || m_minutes != src.m_minutes || m_seconds != src.m_seconds|| m_milliseconds != src.m_milliseconds;
}

/*-------------------------------------------------------------------------*/

CString 
CTime::ToIso8601TimeString( bool includeDelimeters ) const
{GUCEF_TRACE;

    // Dont reimplement, just call the CDateTime version
    return CDateTime( *this, true ).ToIso8601TimeString( includeDelimeters );
}

/*-------------------------------------------------------------------------*/

bool 
CTime::FromIso8601TimeString( const CString& source )
{GUCEF_TRACE;

    // Dont reimplement, just call the CDateTime version
    CDateTime dt;
    if ( dt.FromIso8601TimeString( source ) )
    {
        *this = dt.GetTime();
        return true;    
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
