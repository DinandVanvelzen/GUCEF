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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CTimeZoneRules.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Int64 TZ_MINUTES_PER_DAY = 1440;
static const Int64 TZ_SECONDS_PER_DAY = 86400;

/*-------------------------------------------------------------------------*/

static Int64
TzFloorDiv( Int64 value, Int64 divisor )
{
    Int64 quotient = value / divisor;
    if ( ( value % divisor != 0 ) && ( ( value < 0 ) != ( divisor < 0 ) ) )
        --quotient;
    return quotient;
}

/*-------------------------------------------------------------------------*/

static Int32
TzYearOfDay( Int64 days )
{
    Int64 year = 0;
    Int32 month = 0, day = 0;
    CTimeZoneRules::CivilFromDays( days, year, month, day );
    return (Int32) year;
}

/*-------------------------------------------------------------------------*/

static Int32
TzDaysInMonth( Int32 year, Int32 month )
{
    const Int32 next = 12 == month ? 1 : month + 1;
    const Int32 nextYear = 12 == month ? year + 1 : year;
    return (Int32) ( CTimeZoneRules::DaysFromCivil( nextYear, next, 1 ) - CTimeZoneRules::DaysFromCivil( year, month, 1 ) );
}

/*-------------------------------------------------------------------------*/

static Int32
TzMonthFromName( const CString& text )
{
    static const char* const names[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
    const CString lower = text.Lowercase();
    for ( Int32 i=0; i<12; ++i )
        if ( lower.Length() >= 3 && lower.SubstrFromRange( 0, 3 ) == names[ i ] )
            return i + 1;
    return 0;
}

/*-------------------------------------------------------------------------*/

static Int32
TzWeekdayFromName( const CString& text )
{
    static const char* const names[] = { "sun", "mon", "tue", "wed", "thu", "fri", "sat" };
    const CString lower = text.Lowercase();
    for ( Int32 i=0; i<7; ++i )
        if ( lower.Length() >= 3 && lower.SubstrFromRange( 0, 3 ) == names[ i ] )
            return i;
    return -1;
}

/*-------------------------------------------------------------------------*/

static CString
TzTextOf( const CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsString( CString::Empty, false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimeZoneRules::STransition::STransition( void )
    : month( 1 )
    , kind( DAYKIND_FIXED )
    , weekday( 0 )
    , dayOfMonth( 1 )
    , timeMinutes( 0 )
    , basis( 'w' )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeZoneRules::SRulePeriod::SRulePeriod( void )
    : fromYear( 0 )
    , toYear( 0 )
    , saveMinutes( 60 )
    , start()
    , end()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeZoneRules::SZonePeriod::SZonePeriod( void )
    : fromYear( 0 )
    , toYear( 0 )
    , ruleName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeZoneRules::SZone::SZone( void )
    : standardOffsetMinutes( 0 )
    , periods()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeZoneRules::CTimeZoneRules( void )
    : m_rules()
    , m_zones()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeZoneRules::CTimeZoneRules( const CTimeZoneRules& src )
    : m_rules( src.m_rules )
    , m_zones( src.m_zones )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeZoneRules&
CTimeZoneRules::operator=( const CTimeZoneRules& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_rules = src.m_rules;
        m_zones = src.m_zones;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CTimeZoneRules::~CTimeZoneRules()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTimeZoneRules::Clear( void )
{GUCEF_TRACE;

    m_rules.clear();
    m_zones.clear();
}

/*-------------------------------------------------------------------------*/

Int64
CTimeZoneRules::DaysFromCivil( Int64 year, Int32 month, Int32 day )
{GUCEF_TRACE;

    year -= month <= 2 ? 1 : 0;
    const Int64 era = TzFloorDiv( year, 400 );
    const Int64 yearOfEra = year - era * 400;
    const Int64 dayOfYear = ( 153 * ( month + ( month > 2 ? -3 : 9 ) ) + 2 ) / 5 + day - 1;
    const Int64 dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return era * 146097 + dayOfEra - 719468;
}

/*-------------------------------------------------------------------------*/

void
CTimeZoneRules::CivilFromDays( Int64 days, Int64& year, Int32& month, Int32& day )
{GUCEF_TRACE;

    days += 719468;
    const Int64 era = TzFloorDiv( days, 146097 );
    const Int64 dayOfEra = days - era * 146097;
    const Int64 yearOfEra = ( dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096 ) / 365;
    const Int64 dayOfYear = dayOfEra - ( 365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100 );
    const Int64 monthPrime = ( 5 * dayOfYear + 2 ) / 153;
    day = (Int32) ( dayOfYear - ( 153 * monthPrime + 2 ) / 5 + 1 );
    month = (Int32) ( monthPrime + ( monthPrime < 10 ? 3 : -9 ) );
    year = yearOfEra + era * 400 + ( month <= 2 ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

Int32
CTimeZoneRules::WeekdayOfDay( Int64 days )
{GUCEF_TRACE;

    // 1970-01-01 was a Thursday; 0 = Sunday
    Int64 weekday = ( days + 4 ) % 7;
    if ( weekday < 0 )
        weekday += 7;
    return (Int32) weekday;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::ParseTransition( const CString& text, STransition& transition )
{GUCEF_TRACE;

    // "Mar Sun>=8 2:00", "Oct lastSun 1:00u", "Apr 15 0:00s"
    const CString::StringVector parts = text.Trim( true ).Trim( false ).ParseElements( ' ', false );
    CString::StringVector tokens;
    for ( size_t i=0; i<parts.size(); ++i )
        if ( !parts[ i ].IsNULLOrEmpty() )
            tokens.push_back( parts[ i ] );
    if ( 3 != tokens.size() )
        return false;

    STransition parsed;
    parsed.month = TzMonthFromName( tokens[ 0 ] );
    if ( 0 == parsed.month )
        return false;

    const CString day = tokens[ 1 ];
    if ( day.Length() > 4 && day.SubstrFromRange( 0, 4 ).Lowercase() == "last" )
    {
        parsed.kind = DAYKIND_LAST;
        parsed.weekday = TzWeekdayFromName( day.SubstrFromRange( 4, day.Length() ) );
        if ( parsed.weekday < 0 )
            return false;
    }
    else if ( day.HasSubstr( ">=" ) > 0 || day.HasSubstr( "<=" ) > 0 )
    {
        const bool after = day.HasSubstr( ">=" ) > 0;
        const Int32 at = day.HasSubstr( after ? ">=" : "<=" );
        parsed.kind = after ? DAYKIND_ON_OR_AFTER : DAYKIND_ON_OR_BEFORE;
        parsed.weekday = TzWeekdayFromName( day.SubstrFromRange( 0, (UInt32) at ) );
        parsed.dayOfMonth = StringToInt32( day.SubstrFromRange( (UInt32) at + 2, day.Length() ), 0 );
        if ( parsed.weekday < 0 || parsed.dayOfMonth < 1 || parsed.dayOfMonth > 31 )
            return false;
    }
    else
    {
        parsed.kind = DAYKIND_FIXED;
        parsed.dayOfMonth = StringToInt32( day, 0 );
        if ( parsed.dayOfMonth < 1 || parsed.dayOfMonth > 31 )
            return false;
    }

    CString timeText = tokens[ 2 ];
    if ( timeText.Length() > 1 )
    {
        const char last = timeText[ timeText.Length() - 1 ];
        if ( 'w' == last || 's' == last || 'u' == last )
        {
            parsed.basis = last;
            timeText = timeText.SubstrFromRange( 0, timeText.Length() - 1 );
        }
    }
    const Int32 colon = timeText.HasChar( ':', true );
    if ( colon < 1 )
        return false;
    const Int32 hours = StringToInt32( timeText.SubstrFromRange( 0, (UInt32) colon ), -1 );
    const Int32 minutes = StringToInt32( timeText.SubstrFromRange( (UInt32) colon + 1, timeText.Length() ), -1 );
    if ( hours < 0 || hours > 24 || minutes < 0 || minutes > 59 )
        return false;
    parsed.timeMinutes = hours * 60 + minutes;
    transition = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::ParseRules( const CDataNode& root, TRuleMap& rules, CString& error )
{GUCEF_TRACE;

    const CDataNode* ruleNodes = root.FindChild( "rules" );
    if ( GUCEF_NULL == ruleNodes )
        return true;

    for ( CDataNode::const_iterator r = ruleNodes->ConstBegin(); r != ruleNodes->ConstEnd(); ++r )
    {
        const CString name = TzTextOf( *(*r), "name" );
        const CDataNode* periods = (*r)->FindChild( "periods" );
        if ( name.IsNULLOrEmpty() || GUCEF_NULL == periods )
        {
            error = "A time zone rule needs a name and periods";
            return false;
        }
        TRulePeriodVector parsedPeriods;
        for ( CDataNode::const_iterator p = periods->ConstBegin(); p != periods->ConstEnd(); ++p )
        {
            SRulePeriod period;
            period.fromYear = (*p)->GetAttributeValueOrChildValueByName( "fromYear" ).AsInt32( 0, false );
            period.toYear = (*p)->GetAttributeValueOrChildValueByName( "toYear" ).AsInt32( 0, false );
            period.saveMinutes = (*p)->GetAttributeValueOrChildValueByName( "saveMinutes" ).AsInt32( 60, false );
            if ( period.fromYear < 1 || period.toYear < period.fromYear || period.saveMinutes < 1 || period.saveMinutes > 12 * 60 ||
                 !ParseTransition( TzTextOf( *(*p), "start" ), period.start ) || !ParseTransition( TzTextOf( *(*p), "end" ), period.end ) )
            {
                error = "The rule " + name + " has a period that is not valid";
                return false;
            }
            parsedPeriods.push_back( period );
        }
        rules[ name ] = parsedPeriods;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::ParseZones( const CDataNode& root, const TRuleMap& rules, TZoneMap& zones, CString& error )
{GUCEF_TRACE;

    const CDataNode* zoneNodes = root.FindChild( "zones" );
    if ( GUCEF_NULL == zoneNodes )
        return true;

    for ( CDataNode::const_iterator z = zoneNodes->ConstBegin(); z != zoneNodes->ConstEnd(); ++z )
    {
        const CString name = TzTextOf( *(*z), "name" );
        const CDataNode* periods = (*z)->FindChild( "periods" );
        if ( name.IsNULLOrEmpty() || GUCEF_NULL == periods )
        {
            error = "A time zone needs a name and periods";
            return false;
        }
        SZone zone;
        zone.standardOffsetMinutes = (*z)->GetAttributeValueOrChildValueByName( "standardOffsetMinutes" ).AsInt32( 0, false );
        if ( zone.standardOffsetMinutes < -14 * 60 || zone.standardOffsetMinutes > 14 * 60 )
        {
            error = "The zone " + name + " has an offset that is not valid";
            return false;
        }
        for ( CDataNode::const_iterator p = periods->ConstBegin(); p != periods->ConstEnd(); ++p )
        {
            SZonePeriod period;
            period.fromYear = (*p)->GetAttributeValueOrChildValueByName( "fromYear" ).AsInt32( 0, false );
            period.toYear = (*p)->GetAttributeValueOrChildValueByName( "toYear" ).AsInt32( 0, false );
            period.ruleName = TzTextOf( *(*p), "rule" );
            if ( period.ruleName == "NONE" )
                period.ruleName.Clear();
            if ( period.fromYear < 1 || period.toYear < period.fromYear || ( !period.ruleName.IsNULLOrEmpty() && rules.find( period.ruleName ) == rules.end() ) )
            {
                error = "The zone " + name + " has a period that is not valid or names an unknown rule";
                return false;
            }
            zone.periods.push_back( period );
        }
        zones[ name ] = zone;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::LoadFromDataNode( const CDataNode& root, CString& error )
{GUCEF_TRACE;

    // Parse into copies: the object is only changed when everything is valid
    TRuleMap rules = m_rules;
    TZoneMap zones = m_zones;
    if ( !ParseRules( root, rules, error ) || !ParseZones( root, rules, zones, error ) )
        return false;
    m_rules.swap( rules );
    m_zones.swap( zones );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::LoadFromIOAccess( CIOAccess& access, const CString& codecTypeName, CString& error )
{GUCEF_TRACE;

    if ( codecTypeName.IsNULLOrEmpty() )
    {
        error = "No data store codec type was given for the time zone data";
        return false;
    }
    CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecTypeName, codec, false ) || codec.IsNULL() )
    {
        error = "No data store codec is available for the type " + codecTypeName;
        return false;
    }
    CDataNode data;
    if ( !codec->BuildDataTree( &data, &access ) )
    {
        error = "The time zone data cannot be read with the codec " + codecTypeName;
        return false;
    }
    return LoadFromDataNode( data, error );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::LoadFromFile( const CString& filePath, CString& error )
{GUCEF_TRACE;

    CFileAccess access( filePath, "rb" );
    if ( !access.IsValid() )
    {
        error = "The time zone file " + filePath + " cannot be opened";
        return false;
    }
    return LoadFromIOAccess( access, ExtractFileExtention( filePath ), error );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::FindZone( const CString& zoneName, const SZone*& zone ) const
{GUCEF_TRACE;

    TZoneMap::const_iterator i = m_zones.find( zoneName );
    if ( i == m_zones.end() )
        return false;
    zone = &(*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::HasZone( const CString& zoneName ) const
{GUCEF_TRACE;

    return m_zones.find( zoneName ) != m_zones.end();
}

/*-------------------------------------------------------------------------*/

void
CTimeZoneRules::GetZoneNames( CString::StringVector& names ) const
{GUCEF_TRACE;

    for ( TZoneMap::const_iterator i = m_zones.begin(); i != m_zones.end(); ++i )
        names.push_back( (*i).first );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetStandardOffsetMinutes( const CString& zoneName, Int32& offsetMinutes ) const
{GUCEF_TRACE;

    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) )
        return false;
    offsetMinutes = zone->standardOffsetMinutes;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::IsYearKnown( const CString& zoneName, Int32 year ) const
{GUCEF_TRACE;

    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) )
        return false;
    bool hasRule = false;
    FindRulePeriod( *zone, year, hasRule );
    return hasRule;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetKnownYearRange( const CString& zoneName, Int32& firstYear, Int32& lastYear ) const
{GUCEF_TRACE;

    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) || zone->periods.empty() )
        return false;
    firstYear = zone->periods[ 0 ].fromYear;
    lastYear = zone->periods[ 0 ].toYear;
    for ( size_t p=1; p<zone->periods.size(); ++p )
    {
        if ( zone->periods[ p ].fromYear < firstYear )
            firstYear = zone->periods[ p ].fromYear;
        if ( zone->periods[ p ].toYear > lastYear )
            lastYear = zone->periods[ p ].toYear;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int64
CTimeZoneRules::TransitionMinutes( const STransition& transition, Int32 year, Int32 standardOffsetMinutes, Int32 offsetBeforeMinutes )
{GUCEF_TRACE;

    const Int64 first = DaysFromCivil( year, transition.month, 1 );
    Int64 day = first;
    switch ( transition.kind )
    {
        case DAYKIND_FIXED:
        {
            day = first + transition.dayOfMonth - 1;
            break;
        }
        case DAYKIND_LAST:
        {
            day = first + TzDaysInMonth( year, transition.month ) - 1;
            day -= ( WeekdayOfDay( day ) - transition.weekday + 7 ) % 7;
            break;
        }
        case DAYKIND_ON_OR_AFTER:
        {
            day = first + transition.dayOfMonth - 1;
            day += ( transition.weekday - WeekdayOfDay( day ) + 7 ) % 7;
            break;
        }
        case DAYKIND_ON_OR_BEFORE:
        {
            day = first + transition.dayOfMonth - 1;
            day -= ( WeekdayOfDay( day ) - transition.weekday + 7 ) % 7;
            break;
        }
    }
    const Int64 local = day * TZ_MINUTES_PER_DAY + transition.timeMinutes;
    switch ( transition.basis )
    {
        case 'u':   return local;
        case 's':   return local - standardOffsetMinutes;
        default:    return local - offsetBeforeMinutes;
    }
}

/*-------------------------------------------------------------------------*/

const CTimeZoneRules::SRulePeriod*
CTimeZoneRules::FindRulePeriod( const SZone& zone, Int32 year, bool& hasRule ) const
{GUCEF_TRACE;

    hasRule = false;
    for ( size_t p=0; p<zone.periods.size(); ++p )
    {
        const SZonePeriod& zonePeriod = zone.periods[ p ];
        if ( year < zonePeriod.fromYear || year > zonePeriod.toYear )
            continue;
        hasRule = true;
        if ( zonePeriod.ruleName.IsNULLOrEmpty() )
            return GUCEF_NULL;
        TRuleMap::const_iterator rule = m_rules.find( zonePeriod.ruleName );
        if ( rule == m_rules.end() )
            return GUCEF_NULL;
        for ( size_t r=0; r<(*rule).second.size(); ++r )
        {
            const SRulePeriod& period = (*rule).second[ r ];
            if ( year >= period.fromYear && year <= period.toYear )
                return &period;
        }
        hasRule = false;        // the zone says a rule applies, the rule has nothing for this year
        return GUCEF_NULL;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::OffsetAtMinute( const SZone& zone, Int64 utcMinute, Int32& offsetMinutes, bool* isDaylightSaving ) const
{GUCEF_TRACE;

    const Int32 year = TzYearOfDay( TzFloorDiv( utcMinute + zone.standardOffsetMinutes, TZ_MINUTES_PER_DAY ) );
    bool hasRule = false;
    const SRulePeriod* period = FindRulePeriod( zone, year, hasRule );
    if ( !hasRule )
        return false;

    bool daylight = false;
    if ( GUCEF_NULL != period )
    {
        const Int64 start = TransitionMinutes( period->start, year, zone.standardOffsetMinutes, zone.standardOffsetMinutes );
        const Int64 end = TransitionMinutes( period->end, year, zone.standardOffsetMinutes, zone.standardOffsetMinutes + period->saveMinutes );
        if ( start < end )
            daylight = utcMinute >= start && utcMinute < end;
        else
            daylight = utcMinute >= start || utcMinute < end;
        offsetMinutes = zone.standardOffsetMinutes + ( daylight ? period->saveMinutes : 0 );
    }
    else
    {
        offsetMinutes = zone.standardOffsetMinutes;
    }
    if ( GUCEF_NULL != isDaylightSaving )
        *isDaylightSaving = daylight;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::ToUtcSeconds( const CDateTime& dateTime, Int64& seconds, Int32& milliseconds )
{GUCEF_TRACE;

    const Int32 month = dateTime.GetMonth();
    const Int32 day = dateTime.GetDay();
    const Int32 hours = dateTime.GetHours();
    const Int32 minutes = dateTime.GetMinutes();
    const Int32 secs = dateTime.GetSeconds();
    const Int32 millis = dateTime.GetMilliseconds();
    if ( month < 1 || month > 12 || day < 1 || day > TzDaysInMonth( dateTime.GetYear(), month ) ||
         hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || secs < 0 || secs > 59 || millis < 0 || millis > 999 )
        return false;

    seconds = DaysFromCivil( dateTime.GetYear(), month, day ) * TZ_SECONDS_PER_DAY + hours * 3600 + minutes * 60 + secs;
    seconds -= (Int64) dateTime.GetTimeZoneUTCOffsetInMins() * 60;
    milliseconds = millis;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::FromUtcSeconds( Int64 seconds, Int32 milliseconds, Int32 offsetMinutes, CDateTime& dateTime )
{GUCEF_TRACE;

    const Int64 local = seconds + (Int64) offsetMinutes * 60;
    const Int64 days = TzFloorDiv( local, TZ_SECONDS_PER_DAY );
    const Int64 rest = local - days * TZ_SECONDS_PER_DAY;
    Int64 year = 0;
    Int32 month = 0, day = 0;
    CivilFromDays( days, year, month, day );
    if ( year < -32768 || year > 32767 )
        return false;
    dateTime.Set( (Int16) year, (UInt8) month, (UInt8) day, (Int8) ( rest / 3600 ), (Int8) ( ( rest / 60 ) % 60 ), (Int8) ( rest % 60 ), (Int16) milliseconds, (Int16) offsetMinutes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetOffsetAtInstant( const CString& zoneName, Int64 utcSeconds, Int32& offsetMinutes, bool* isDaylightSaving ) const
{GUCEF_TRACE;

    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) )
        return false;
    return OffsetAtMinute( *zone, TzFloorDiv( utcSeconds, 60 ), offsetMinutes, isDaylightSaving );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetOffsetAtInstant( const CString& zoneName, const CDateTime& instant, Int32& offsetMinutes, bool* isDaylightSaving ) const
{GUCEF_TRACE;

    Int64 utcSeconds = 0;
    Int32 milliseconds = 0;
    return ToUtcSeconds( instant, utcSeconds, milliseconds ) && GetOffsetAtInstant( zoneName, utcSeconds, offsetMinutes, isDaylightSaving );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetLocalDay( const CString& zoneName, Int64 utcSeconds, Int32& day ) const
{GUCEF_TRACE;

    Int32 offset = 0;
    if ( !GetOffsetAtInstant( zoneName, utcSeconds, offset, GUCEF_NULL ) )
        return false;
    day = (Int32) TzFloorDiv( utcSeconds + (Int64) offset * 60, TZ_SECONDS_PER_DAY );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetLocalDate( const CString& zoneName, Int64 utcSeconds, CDate& date ) const
{GUCEF_TRACE;

    Int32 day = 0;
    if ( !GetLocalDay( zoneName, utcSeconds, day ) )
        return false;
    Int64 year = 0;
    Int32 month = 0, dayOfMonth = 0;
    CivilFromDays( day, year, month, dayOfMonth );
    if ( year < 1 || year > 9999 )
        return false;
    date = CDate( (UInt16) year, (UInt8) month, (UInt8) dayOfMonth );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::UtcToLocal( const CString& zoneName, Int64 utcSeconds, CDateTime& local ) const
{GUCEF_TRACE;

    Int32 offset = 0;
    return GetOffsetAtInstant( zoneName, utcSeconds, offset, GUCEF_NULL ) && FromUtcSeconds( utcSeconds, 0, offset, local );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::UtcToLocal( const CString& zoneName, const CDateTime& instant, CDateTime& local ) const
{GUCEF_TRACE;

    Int64 utcSeconds = 0;
    Int32 milliseconds = 0;
    Int32 offset = 0;
    return ToUtcSeconds( instant, utcSeconds, milliseconds ) &&
           GetOffsetAtInstant( zoneName, utcSeconds, offset, GUCEF_NULL ) &&
           FromUtcSeconds( utcSeconds, milliseconds, offset, local );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::ResolveLocalMinute( const SZone& zone                         ,
                                    Int64 localMinute                         ,
                                    TAmbiguousTimePolicy ambiguousPolicy      ,
                                    TNonexistentTimePolicy nonexistentPolicy  ,
                                    Int64& utcMinute                          ,
                                    TLocalTimeKind& kind                      ) const
{GUCEF_TRACE;

    // The wall clock time maps to UTC with the offset in force around it: the offsets of the day before and the day after are the candidates.
    // A candidate counts when the offset really is in force at the instant it gives.
    Int32 before = 0, after = 0;
    if ( !OffsetAtMinute( zone, localMinute - zone.standardOffsetMinutes - TZ_MINUTES_PER_DAY, before, GUCEF_NULL ) ||
         !OffsetAtMinute( zone, localMinute - zone.standardOffsetMinutes + TZ_MINUTES_PER_DAY, after, GUCEF_NULL ) )
        return false;

    Int64 valid[ 2 ];
    UInt32 validCount = 0;
    const Int32 candidates[ 2 ] = { before, after };
    for ( int c=0; c<2; ++c )
    {
        if ( 1 == c && before == after )
            break;
        const Int64 utc = localMinute - candidates[ c ];
        Int32 actual = 0;
        if ( OffsetAtMinute( zone, utc, actual, GUCEF_NULL ) && actual == candidates[ c ] )
            valid[ validCount++ ] = utc;
    }

    if ( 1 == validCount )
    {
        kind = LOCALTIME_UNIQUE;
        utcMinute = valid[ 0 ];
        return true;
    }
    if ( 2 == validCount )
    {
        kind = LOCALTIME_AMBIGUOUS;
        const Int64 earlier = valid[ 0 ] < valid[ 1 ] ? valid[ 0 ] : valid[ 1 ];
        const Int64 later = valid[ 0 ] < valid[ 1 ] ? valid[ 1 ] : valid[ 0 ];
        switch ( ambiguousPolicy )
        {
            case AMBIGUOUSTIME_EARLIER: utcMinute = earlier; return true;
            case AMBIGUOUSTIME_LATER:   utcMinute = later; return true;
            default:                    return false;
        }
    }

    kind = LOCALTIME_NONEXISTENT;
    switch ( nonexistentPolicy )
    {
        case NONEXISTENTTIME_SHIFT_FORWARD:     utcMinute = localMinute - before; return true;
        case NONEXISTENTTIME_SHIFT_BACKWARD:    utcMinute = localMinute - after; return true;
        default:                                return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::LocalToUtc( const CString& zoneName                ,
                            Int64 localSeconds                     ,
                            TAmbiguousTimePolicy ambiguousPolicy   ,
                            TNonexistentTimePolicy nonexistentPolicy ,
                            Int64& utcSeconds                      ,
                            TLocalTimeKind* kind                   ) const
{GUCEF_TRACE;

    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) )
        return false;
    const Int64 localMinute = TzFloorDiv( localSeconds, 60 );
    Int64 utcMinute = 0;
    TLocalTimeKind resolvedKind = LOCALTIME_UNIQUE;
    const bool resolved = ResolveLocalMinute( *zone, localMinute, ambiguousPolicy, nonexistentPolicy, utcMinute, resolvedKind );
    if ( GUCEF_NULL != kind )
        *kind = resolvedKind;
    if ( resolved )
        utcSeconds = utcMinute * 60 + ( localSeconds - localMinute * 60 );
    return resolved;
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::LocalToUtc( const CString& zoneName                ,
                            const CDateTime& local                 ,
                            TAmbiguousTimePolicy ambiguousPolicy   ,
                            TNonexistentTimePolicy nonexistentPolicy ,
                            CDateTime& utc                         ,
                            TLocalTimeKind* kind                   ) const
{GUCEF_TRACE;

    // The wall clock reading is what counts: read it as an instant with no offset
    CDateTime reading( local );
    reading.SetTimeZoneUTCOffsetInMins( 0 );
    Int64 localSeconds = 0;
    Int32 milliseconds = 0;
    Int64 utcSeconds = 0;
    return ToUtcSeconds( reading, localSeconds, milliseconds ) &&
           LocalToUtc( zoneName, localSeconds, ambiguousPolicy, nonexistentPolicy, utcSeconds, kind ) &&
           FromUtcSeconds( utcSeconds, milliseconds, 0, utc );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetLocalTimeKind( const CString& zoneName, Int64 localSeconds, TLocalTimeKind& kind ) const
{GUCEF_TRACE;

    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) )
        return false;
    // with these policies the resolution only fails when the zone has no rule for the years around the local time
    Int64 utcMinute = 0;
    return ResolveLocalMinute( *zone, TzFloorDiv( localSeconds, 60 ), AMBIGUOUSTIME_EARLIER, NONEXISTENTTIME_SHIFT_FORWARD, utcMinute, kind );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetLocalDayStart( const CString& zoneName, Int32 day, Int64& utcSeconds ) const
{GUCEF_TRACE;

    // The first instant of the day: the earlier of the instants of local midnight, or when midnight does not exist (a transition skips it) the first
    // instant after the gap, which is midnight read with the offset before it.
    return LocalToUtc( zoneName, (Int64) day * TZ_SECONDS_PER_DAY, AMBIGUOUSTIME_EARLIER, NONEXISTENTTIME_SHIFT_FORWARD, utcSeconds, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetLocalDayBounds( const CString& zoneName, Int32 day, Int64& startUtcSeconds, Int64& endUtcSeconds ) const
{GUCEF_TRACE;

    return GetLocalDayStart( zoneName, day, startUtcSeconds ) && GetLocalDayStart( zoneName, day + 1, endUtcSeconds );
}

/*-------------------------------------------------------------------------*/

bool
CTimeZoneRules::GetTransitionsOfYear( const CString& zoneName, Int32 year, bool& hasDaylightSaving, Int64& daylightStartUtcSeconds, Int64& daylightEndUtcSeconds ) const
{GUCEF_TRACE;

    hasDaylightSaving = false;
    const SZone* zone = GUCEF_NULL;
    if ( !FindZone( zoneName, zone ) )
        return false;
    bool hasRule = false;
    const SRulePeriod* period = FindRulePeriod( *zone, year, hasRule );
    if ( !hasRule )
        return false;
    if ( GUCEF_NULL == period )
        return true;
    hasDaylightSaving = true;
    daylightStartUtcSeconds = TransitionMinutes( period->start, year, zone->standardOffsetMinutes, zone->standardOffsetMinutes ) * 60;
    daylightEndUtcSeconds = TransitionMinutes( period->end, year, zone->standardOffsetMinutes, zone->standardOffsetMinutes + period->saveMinutes ) * 60;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
