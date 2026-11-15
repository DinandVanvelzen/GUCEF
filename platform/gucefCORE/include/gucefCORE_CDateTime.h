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

#ifndef GUCEF_CORE_CDATETIME_H
#define GUCEF_CORE_CDATETIME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <time.h>

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#ifndef GUCEF_CORE_CTIME_H
#include "gucefCORE_CTime.h"
#define GUCEF_CORE_CTIME_H
#endif /* GUCEF_CORE_CTIME_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

struct tm;

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDynamicBuffer;

class GUCEF_CORE_PUBLIC_CPP CDateTime : public CDate ,
                                        public CTime
{
    public:

    enum EDateTimeComponent
    {
        DATETIMECOMPONENT_UNKNOWN = 0,

        DATETIMECOMPONENT_YEAR,
        DATETIMECOMPONENT_MONTH,
        DATETIMECOMPONENT_DAY,
        DATETIMECOMPONENT_HOURS,
        DATETIMECOMPONENT_MINUTES,
        DATETIMECOMPONENT_SECONDS,
        DATETIMECOMPONENT_MILLISECONDS,
        DATETIMECOMPONENT_TIMEZONEOFFSETINMINS
    };
    typedef enum EDateTimeComponent TDateTimeComponent;

    static const CDateTime Empty;
    static const CDateTime PastMax;
    static const CDateTime FutureMax;

    static CDateTime NowLocalDateTime( void );

    static CDateTime NowUTCDateTime( void );

    static CDateTime UnixEpochDateTime( void );

    static CDateTime WindowsNTEpochDateTime( void );

    static CDateTime CompileDateTime( const char* __date__Macro = __DATE__, const char* __time__Macro = __TIME__ );

    void SetTimeZoneUTCOffsetInMins( Int16 tzOffsetInMins );

    Int16 GetTimeZoneUTCOffsetInMins( void ) const;

    bool IsUTC( void ) const;

    CDateTime ToUTC( void ) const;

    /**
     *  The same instant in the time zone of the operating system the process runs on, with the UTC offset that zone
     *  has at that instant (daylight saving time included). For display only: models, tables and protocols stay UTC
     */
    CDateTime ToSystemLocal( void ) const;

    CDate& GetDate( void );

    const CDate& GetDate( void ) const;

    CTime& GetTime( void );

    const CTime& GetTime( void ) const;

    Int64 GetTimeDifferenceInMillisecondsTowards( const CDateTime& other ) const;

    Int64 GetTimeDifferenceInMillisecondsToNow( void ) const;

    CDateTime& operator=( const CDateTime& src );

    bool operator==( const CDateTime& other ) const;

    bool operator!=( const CDateTime& other ) const;

    bool operator>( const CDateTime& other ) const;

    bool operator<( const CDateTime& other ) const;

    bool operator>=( const CDateTime& other ) const;

    bool operator<=( const CDateTime& other ) const;

    Int32 Compare( const CDateTime& other ) const;

    bool IsWithinRange( const CDateTime& a, const CDateTime& b, bool includingBoundaryItself = true ) const;

    CString ToIso8601DateTimeString( bool includeDelimeters, bool includeMilliseconds ) const;
    Int32 ToIso8601DateTimeString( void* targetBuffer, UInt32 targetBufferSize, bool includeDelimeters, bool includeMilliseconds ) const;
    Int32 ToIso8601DateString( void* targetBuffer, UInt32 targetBufferSize, bool includeDelimeters ) const;
    Int32 ToIso8601TimeString( void* targetBuffer, UInt32 targetBufferSize, bool includeDelimeters ) const;
    Int32 ToIso8601DateTimeString( CDynamicBuffer& target, UInt32 targetBufferOffset, bool includeDelimeters, bool includeMilliseconds ) const;
    virtual CString ToIso8601DateString( bool includeDelimeters ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString ToIso8601TimeString( bool includeDelimeters ) const GUCEF_VIRTUAL_OVERRIDE;

    Int32 FromIso8601DateTimeString( const void* sourceBuffer, UInt32 sourceBufferSize );
    Int32 FromIso8601DateTimeString( const CDynamicBuffer& source, UInt32 sourceBufferOffset = 0 );
    bool FromIso8601DateTimeString( const CString& source );
    virtual bool FromIso8601TimeString( const CString& source ) GUCEF_VIRTUAL_OVERRIDE;

    UInt64 ToUnixEpochBasedTicksInMillisecs( void ) const;
    void FromUnixEpochBasedTicksInMillisecs( UInt64 unixDtInMsTicks );

    void FromTmStruct( const struct tm* tmStruct, bool isUtc );

    void Set( Int16 year                 = 0 ,
              UInt8 month                = 0 ,
              UInt8 day                  = 0 ,
              Int8 hours                 = 0 ,
              Int8 minutes               = 0 ,
              Int8 seconds               = 0 ,
              Int16 milliseconds         = 0 ,
              Int16 timezoneOffsetInMins = 0 );

    void Clear( void );

    void AddSeconds( Int32 secondsToAdd );
    void AddMinutes( Int32 minutesToAdd );
    void AddHours( Int32 hoursToAdd );
    void AddTime( const CTime& timeToAdd );
    void AddDays( Int32 daysToAdd );
    void AddWeeks( Int32 weeksToAdd );

    bool ClampToBoundary( TDateTimeComponent boundary , 
                          TBoundaryType boundaryType  ,
                          CORE::CDateTime& clampedDt  ) const;

    static TDateTimeComponent ToDateTimeComponentType( const CORE::CString& dateTimeComponentType );
    
    explicit CDateTime( const struct tm* src, bool isUtc );

    explicit CDateTime( const time_t src, bool isUtc );

    explicit CDateTime( const timespec& src, bool isUtc );

    explicit CDateTime( const CDate& src, bool isUtc );

    explicit CDateTime( const CTime& src, bool isUtc );

    CDateTime( const CDate& dateSrc, const CTime& timeSrc, Int16 timezoneOffsetInMins = 0 );

    CDateTime( const CDateTime& src );

    CDateTime( Int16 year                 = 0 ,
               UInt8 month                = 0 ,
               UInt8 day                  = 0 ,
               Int8 hours                 = 0 ,
               Int8 minutes               = 0 ,
               Int8 seconds               = 0 ,
               Int16 milliseconds         = 0 ,
               Int16 timezoneOffsetInMins = 0 );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    explicit CDateTime( const struct _FILETIME& src );

    struct _FILETIME ToWindowsFiletime( void ) const;
    
    #endif

    virtual ~CDateTime();

    private:

    Int16 m_timezoneOffsetInMins;
};

/*-------------------------------------------------------------------------*/

inline CString ToString( const CDateTime& value ) { return value.ToIso8601DateTimeString( true, true ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATETIME_H ? */
