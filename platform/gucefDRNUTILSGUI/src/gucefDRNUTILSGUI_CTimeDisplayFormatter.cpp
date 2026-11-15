/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_CTIMEZONERULES_H
#include "gucefCORE_CTimeZoneRules.h"
#define GUCEF_CORE_CTIMEZONERULES_H
#endif /* GUCEF_CORE_CTIMEZONERULES_H ? */

#include "gucefDRNUTILSGUI_CTimeDisplayFormatter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimeDisplayFormatter::CTimeDisplayFormatter( void )
    : m_settings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeDisplayFormatter::CTimeDisplayFormatter( const CTimeDisplaySettings& settings )
    : m_settings( settings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTimeDisplayFormatter::SetSettings( const CTimeDisplaySettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CTimeDisplaySettings&
CTimeDisplayFormatter::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

namespace {

Int64
FloorDiv( Int64 a, Int64 b )
{
    Int64 q = a / b;
    if ( ( a % b != 0 ) && ( ( a < 0 ) != ( b < 0 ) ) )
        --q;
    return q;
}

/** A date and time with the offset zero from seconds since the Unix epoch */
CORE::CDateTime
MakeUtcDateTime( Int64 utcSeconds, Int32 milliseconds )
{
    Int64 days = FloorDiv( utcSeconds, 86400 );
    Int64 secondOfDay = utcSeconds - days * 86400;
    Int64 year = 0;
    Int32 month = 0, day = 0;
    CORE::CTimeZoneRules::CivilFromDays( days, year, month, day );
    return CORE::CDateTime( (Int16) year, (UInt8) month, (UInt8) day, (Int8) ( secondOfDay / 3600 ), (Int8) ( ( secondOfDay % 3600 ) / 60 ),
                            (Int8) ( secondOfDay % 60 ), (Int16) milliseconds, 0 );
}

/** The wall clock reading of a date and time as seconds, as if it were UTC (the offset is not applied) */
Int64
WallSecondsOf( const CORE::CDateTime& dateTime )
{
    return CORE::CTimeZoneRules::DaysFromCivil( dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay() ) * 86400 +
           (Int64) dateTime.GetHours() * 3600 + (Int64) dateTime.GetMinutes() * 60 + dateTime.GetSeconds();
}

CORE::CString
OffsetText( Int32 offsetMinutes )
{
    if ( 0 == offsetMinutes )
        return "UTC";
    char buffer[ 24 ];
    const Int32 absolute = offsetMinutes < 0 ? -offsetMinutes : offsetMinutes;
    sprintf( buffer, "UTC%c%02d:%02d", offsetMinutes < 0 ? '-' : '+', (int) ( absolute / 60 ), (int) ( absolute % 60 ) );
    return buffer;
}

bool
IsZone( const CORE::CString& configured, const char* name )
{
    return configured.Lowercase() == CORE::CString( name );
}

}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplayFormatter::ToDisplayDateTime( Int64 utcMs, CORE::CDateTime& shown ) const
{GUCEF_TRACE;

    const Int64 seconds = FloorDiv( utcMs, 1000 );
    const Int32 milliseconds = (Int32) ( utcMs - seconds * 1000 );
    const CORE::CString& zone = m_settings.GetDisplayTimeZone();

    if ( IsZone( zone, CTimeDisplaySettings::UtcZoneName ) )
    {
        shown = MakeUtcDateTime( seconds, milliseconds );
        return true;
    }
    if ( IsZone( zone, CTimeDisplaySettings::LocalZoneName ) )
    {
        shown = MakeUtcDateTime( seconds, milliseconds ).ToSystemLocal();
        return true;
    }

    const CORE::CTimeZoneRules* rules = m_settings.GetTimeZoneRules();
    if ( GUCEF_NULL != rules && rules->UtcToLocal( zone, seconds, shown ) )
    {
        shown.SetMilliseconds( (Int16) milliseconds );
        return true;
    }

    // never a wrong time without saying so: the suffix of this fallback is UTC
    shown = MakeUtcDateTime( seconds, milliseconds );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplayFormatter::IsDisplayZoneKnown( void ) const
{GUCEF_TRACE;

    const CORE::CString& zone = m_settings.GetDisplayTimeZone();
    if ( IsZone( zone, CTimeDisplaySettings::UtcZoneName ) || IsZone( zone, CTimeDisplaySettings::LocalZoneName ) )
        return true;
    const CORE::CTimeZoneRules* rules = m_settings.GetTimeZoneRules();
    return GUCEF_NULL != rules && rules->HasZone( zone );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTimeDisplayFormatter::FormatPattern( const CORE::CDateTime& shown ) const
{GUCEF_TRACE;

    const CORE::CString& pattern = m_settings.GetDateTimeFormat();
    CORE::CString text;
    char buffer[ 16 ];
    for ( UInt32 i=0; i<pattern.Length(); ++i )
    {
        const char c = pattern[ i ];
        if ( '%' != c || i + 1 >= pattern.Length() )
        {
            text += c;
            continue;
        }
        const char token = pattern[ ++i ];
        switch ( token )
        {
            case 'Y': sprintf( buffer, "%04d", (int) shown.GetYear() ); text += buffer; break;
            case 'm': sprintf( buffer, "%02d", (int) shown.GetMonth() ); text += buffer; break;
            case 'd': sprintf( buffer, "%02d", (int) shown.GetDay() ); text += buffer; break;
            case 'H': sprintf( buffer, "%02d", (int) shown.GetHours() ); text += buffer; break;
            case 'M': sprintf( buffer, "%02d", (int) shown.GetMinutes() ); text += buffer; break;
            case 'S': sprintf( buffer, "%02d", (int) shown.GetSeconds() ); text += buffer; break;
            case 'f': sprintf( buffer, "%03d", (int) shown.GetMilliseconds() ); text += buffer; break;
            case '%': text += '%'; break;
            default: text += '%'; text += token; break;
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTimeDisplayFormatter::FormatUtcMs( Int64 utcMs ) const
{GUCEF_TRACE;

    if ( 0 == utcMs )
        return CORE::CString::Empty;

    CORE::CDateTime shown;
    ToDisplayDateTime( utcMs, shown );
    CORE::CString text = FormatPattern( shown );
    if ( m_settings.GetShowZoneSuffix() )
    {
        text += ' ';
        text += OffsetText( shown.GetTimeZoneUTCOffsetInMins() );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTimeDisplayFormatter::GetZoneSuffix( Int64 utcMs ) const
{GUCEF_TRACE;

    CORE::CDateTime shown;
    ToDisplayDateTime( utcMs, shown );
    return OffsetText( shown.GetTimeZoneUTCOffsetInMins() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTimeDisplayFormatter::GetZoneDescription( Int64 utcMs ) const
{GUCEF_TRACE;

    const CORE::CString& zone = m_settings.GetDisplayTimeZone();
    if ( IsZone( zone, CTimeDisplaySettings::UtcZoneName ) )
        return "UTC";

    CORE::CDateTime shown;
    const bool known = ToDisplayDateTime( utcMs, shown );
    const CORE::CString offset = OffsetText( shown.GetTimeZoneUTCOffsetInMins() );
    if ( IsZone( zone, CTimeDisplaySettings::LocalZoneName ) )
        return "local (" + offset + ")";
    if ( !known )
        return zone + " (unknown zone, shown as UTC)";
    return zone + " (" + offset + ")";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTimeDisplayFormatter::FormatUtcTooltip( Int64 utcMs ) const
{GUCEF_TRACE;

    if ( 0 == utcMs )
        return CORE::CString::Empty;

    const Int64 seconds = FloorDiv( utcMs, 1000 );
    const Int32 milliseconds = (Int32) ( utcMs - seconds * 1000 );
    const CORE::CDateTime utc = MakeUtcDateTime( seconds, milliseconds );
    char buffer[ 48 ];
    if ( 0 != milliseconds )
        sprintf( buffer, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", (int) utc.GetYear(), (int) utc.GetMonth(), (int) utc.GetDay(),
                 (int) utc.GetHours(), (int) utc.GetMinutes(), (int) utc.GetSeconds(), (int) milliseconds );
    else
        sprintf( buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ", (int) utc.GetYear(), (int) utc.GetMonth(), (int) utc.GetDay(),
                 (int) utc.GetHours(), (int) utc.GetMinutes(), (int) utc.GetSeconds() );
    return buffer;
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplayFormatter::WallClockToUtcMs( Int64 wallSeconds, Int32 milliseconds, Int64& utcMs, CORE::CString& error ) const
{GUCEF_TRACE;

    const CORE::CString& zone = m_settings.GetDisplayTimeZone();
    Int64 utcSeconds = wallSeconds;

    if ( IsZone( zone, CTimeDisplaySettings::UtcZoneName ) )
    {
        utcMs = utcSeconds * 1000 + milliseconds;
        return true;
    }

    if ( IsZone( zone, CTimeDisplaySettings::LocalZoneName ) )
    {
        // The operating system only converts an instant to local time, so the instants that show this wall clock reading are looked for:
        // the offsets in force a day before and a day after the reading are the only candidates
        Int32 offsets[ 2 ];
        offsets[ 0 ] = MakeUtcDateTime( wallSeconds - 86400, 0 ).ToSystemLocal().GetTimeZoneUTCOffsetInMins();
        offsets[ 1 ] = MakeUtcDateTime( wallSeconds + 86400, 0 ).ToSystemLocal().GetTimeZoneUTCOffsetInMins();
        Int64 found[ 2 ];
        UInt32 foundCount = 0;
        for ( UInt32 i=0; i<2; ++i )
        {
            const Int64 candidate = wallSeconds - (Int64) offsets[ i ] * 60;
            if ( WallSecondsOf( MakeUtcDateTime( candidate, 0 ).ToSystemLocal() ) != wallSeconds )
                continue;
            if ( foundCount > 0 && found[ 0 ] == candidate )
                continue;
            found[ foundCount++ ] = candidate;
        }
        if ( 0 == foundCount )
        {
            error = "This local time does not exist in the time zone of this computer: the clocks skip it. Enter another time";
            return false;
        }
        if ( foundCount > 1 )
        {
            error = "This local time occurs twice in the time zone of this computer: the clocks go back over it. Enter another time";
            return false;
        }
        utcMs = found[ 0 ] * 1000 + milliseconds;
        return true;
    }

    const CORE::CTimeZoneRules* rules = m_settings.GetTimeZoneRules();
    if ( GUCEF_NULL == rules || !rules->HasZone( zone ) )
    {
        error = "The time zone \"" + zone + "\" is not known: the time cannot be converted";
        return false;
    }
    CORE::CTimeZoneRules::TLocalTimeKind kind = CORE::CTimeZoneRules::LOCALTIME_UNIQUE;
    if ( !rules->LocalToUtc( zone, wallSeconds, CORE::CTimeZoneRules::AMBIGUOUSTIME_REJECT, CORE::CTimeZoneRules::NONEXISTENTTIME_REJECT, utcSeconds, &kind ) )
    {
        if ( CORE::CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind )
            error = "This local time occurs twice in the time zone " + zone + ": the clocks go back over it. Enter another time";
        else
        if ( CORE::CTimeZoneRules::LOCALTIME_NONEXISTENT == kind )
            error = "This local time does not exist in the time zone " + zone + ": the clocks skip it. Enter another time";
        else
            error = "The time zone " + zone + " has no rule for this date: the time cannot be converted";
        return false;
    }
    utcMs = utcSeconds * 1000 + milliseconds;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplayFormatter::ParseDisplayTime( const CORE::CString& text, Int64& utcMs, CORE::CString& error ) const
{GUCEF_TRACE;

    // "YYYY-MM-DD HH:MM", with ":SS" and ".mmm" optionally, a T instead of the space allowed
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, milliseconds = 0;
    int consumed = 0;
    char separator = 0;
    const CORE::CString trimmed = text.Trim( true ).Trim( false );
    const char* input = trimmed.C_String();
    if ( GUCEF_NULL == input || sscanf( input, "%d-%d-%d%c%d:%d%n", &year, &month, &day, &separator, &hour, &minute, &consumed ) < 6 || ( ' ' != separator && 'T' != separator ) )
    {
        error = "The time has to be written as YYYY-MM-DD HH:MM[:SS[.mmm]]";
        return false;
    }
    const char* rest = input + consumed;
    if ( ':' == rest[ 0 ] )
    {
        int more = 0;
        if ( sscanf( rest, ":%d%n", &second, &more ) < 1 )
        {
            error = "The seconds of the time are not a number";
            return false;
        }
        rest += more;
        if ( '.' == rest[ 0 ] )
        {
            int digits = 0;
            if ( sscanf( rest, ".%d%n", &milliseconds, &more ) < 1 )
            {
                error = "The milliseconds of the time are not a number";
                return false;
            }
            digits = more - 1;
            if ( digits != 3 )
            {
                error = "The milliseconds of the time need exactly three digits";
                return false;
            }
            rest += more;
        }
    }
    if ( '\0' != rest[ 0 ] )
    {
        error = "The time has to be written as YYYY-MM-DD HH:MM[:SS[.mmm]]";
        return false;
    }

    static const int daysInMonth[ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    const bool leap = ( 0 == year % 4 && 0 != year % 100 ) || 0 == year % 400;
    if ( month < 1 || month > 12 || day < 1 || day > daysInMonth[ month - 1 ] + ( 2 == month && leap ? 1 : 0 ) ||
         hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59 || milliseconds < 0 || milliseconds > 999 ||
         year < 1 || year > 9999 )
    {
        error = "The time is not a valid date and time";
        return false;
    }

    const Int64 wallSeconds = CORE::CTimeZoneRules::DaysFromCivil( year, month, day ) * 86400 + hour * 3600 + minute * 60 + second;
    return WallClockToUtcMs( wallSeconds, milliseconds, utcMs, error );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
