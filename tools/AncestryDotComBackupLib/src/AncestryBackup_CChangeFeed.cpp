/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#include "AncestryBackup_CUrlTemplate.h"
#define ANCESTRYBACKUP_CURLTEMPLATE_H
#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include "AncestryBackup_CChangeFeed.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

std::string
CChangeFeed::BuildUrl( const std::string& treeId, UInt32 limit, const std::string& cursor )
{GUCEF_TRACE;

    std::string url = "https://www.ancestry.com/tree-change-event/api/change-events?useMockData=false&limit=" + std::to_string( (unsigned long long) limit ) + "&treeId=" + treeId;
    if ( !cursor.empty() )
        url += "&cursor=" + cursor;
    return url;
}

/*-------------------------------------------------------------------------*/

// days since 1970-01-01 of a civil date (Howard Hinnant's algorithm), so that no time zone gets involved
static long long
DaysFromCivil( long long y, unsigned m, unsigned d )
{
    y -= m <= 2;
    const long long era = ( y >= 0 ? y : y - 399 ) / 400;
    const unsigned yoe = (unsigned) ( y - era * 400 );
    const unsigned doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long long) doe - 719468;
}

/*-------------------------------------------------------------------------*/

bool
CChangeFeed::ParseTime( const std::string& text, UInt64& unixEpochInMs )
{GUCEF_TRACE;

    int year, month, day, hour, minute, second;
    char tail[ 32 ] = { 0 };
    if ( sscanf( text.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d%31s", &year, &month, &day, &hour, &minute, &second, tail ) < 6 )
        return false;
    if ( month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60 )
        return false;

    UInt64 fractionInMs = 0;
    if ( tail[ 0 ] == '.' )
    {
        // up to three digits are milliseconds
        unsigned digits = 0;
        unsigned scale = 100;
        for ( const char* c = tail + 1; *c >= '0' && *c <= '9' && digits < 3; ++c, ++digits, scale /= 10 )
            fractionInMs += (UInt64) ( *c - '0' ) * scale;
    }
    else
    if ( tail[ 0 ] != '\0' && tail[ 0 ] != 'Z' )
        return false;       // an offset from UTC is not something the site uses, do not guess

    const long long days = DaysFromCivil( year, (unsigned) month, (unsigned) day );
    if ( days < 0 )
        return false;
    unixEpochInMs = ( (UInt64) days * 86400ULL + (UInt64) hour * 3600ULL + (UInt64) minute * 60ULL + (UInt64) second ) * 1000ULL + fractionInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChangeFeed::ParseGedcomDateTime( const std::string& date, const std::string& time, UInt64& unixEpochInMs )
{GUCEF_TRACE;

    int day = 0, year = 0;
    char monthName[ 16 ] = { 0 };
    if ( sscanf( date.c_str(), "%d %15s %d", &day, monthName, &year ) != 3 )
        return false;
    static const char* const months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec", GUCEF_NULL };
    int month = 0;
    for ( int m=0; months[ m ] != GUCEF_NULL; ++m )
        if ( strlen( monthName ) >= 3 && tolower( monthName[ 0 ] ) == months[ m ][ 0 ] && tolower( monthName[ 1 ] ) == months[ m ][ 1 ] && tolower( monthName[ 2 ] ) == months[ m ][ 2 ] )
            month = m + 1;
    if ( month == 0 || day < 1 || day > 31 || year < 1970 )
        return false;

    int hour = 0, minute = 0, second = 0;
    if ( !time.empty() )
        if ( sscanf( time.c_str(), "%d:%d:%d", &hour, &minute, &second ) < 2 )
            return false;

    char text[ 40 ];
    snprintf( text, sizeof( text ), "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second );
    return ParseTime( text, unixEpochInMs );
}

/*-------------------------------------------------------------------------*/

std::string
CChangeFeed::FormatTime( UInt64 unixEpochInMs )
{GUCEF_TRACE;

    const time_t seconds = (time_t) ( unixEpochInMs / 1000 );
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    gmtime_s( &parts, &seconds );
    #else
    gmtime_r( &seconds, &parts );
    #endif
    char text[ 32 ];
    strftime( text, sizeof( text ), "%Y-%m-%dT%H:%M:%SZ", &parts );
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CChangeFeed::ParsePage( const CORE::CDataNode& document, SChangePage& page )
{GUCEF_TRACE;

    page = SChangePage();
    const CORE::CDataNode* changes = document.Find( "changes" );
    if ( GUCEF_NULL == changes )
        return false;

    for ( CORE::CDataNode::const_iterator i = changes->ConstBegin(); i != changes->ConstEnd(); ++i )
    {
        const CORE::CDataNode& node = *(*i);
        SChangeEvent event;
        event.action = node.GetAttributeValue( "action" ).AsString().STL_String();
        event.time = node.GetAttributeValue( "time" ).AsString().STL_String();
        if ( !ParseTime( event.time, event.timeInMs ) )
            continue;
        event.referenceId = node.GetAttributeValue( "referenceId" ).AsString().STL_String();
        event.personNoLongerExists = node.GetAttributeValue( "personNoLongerExists" ).AsBool( false );

        const CORE::CDataNode* person = node.Find( "person" );
        if ( GUCEF_NULL != person )
            event.personId = person->GetAttributeValue( "personId" ).AsString().STL_String();
        const CORE::CDataNode* fact = node.Find( "fact" );
        if ( GUCEF_NULL != fact )
        {
            event.factId = fact->GetAttributeValue( "id" ).AsString().STL_String();
            event.factType = fact->GetAttributeValue( "type" ).AsString().STL_String();
        }
        const CORE::CDataNode* author = node.Find( "author" );
        if ( GUCEF_NULL != author )
            event.authorId = author->GetAttributeValue( "userId" ).AsString().STL_String();
        page.events.push_back( event );
    }

    page.nextCursor = document.GetAttributeValue( "cursor" ).AsString().STL_String();
    return true;
}

/*-------------------------------------------------------------------------*/

std::vector< std::string >
CChangeFeed::GetChangedPeople( const std::vector< SChangeEvent >& events )
{GUCEF_TRACE;

    std::vector< std::string > people;
    std::set< std::string > seen;
    for ( size_t i=0; i<events.size(); ++i )
        if ( !events[ i ].personId.empty() && seen.insert( events[ i ].personId ).second )
            people.push_back( events[ i ].personId );
    return people;
}

/*-------------------------------------------------------------------------*/

std::string
CChangeFeed::ToJournalLine( const SChangeEvent& event )
{GUCEF_TRACE;

    return "{\"time\":\"" + event.time + "\",\"action\":\"" + CUrlTemplate::EscapeJson( event.action.c_str() ).STL_String() + "\",\"personId\":\"" + event.personId +
           "\",\"factId\":\"" + CUrlTemplate::EscapeJson( event.factId.c_str() ).STL_String() + "\",\"factType\":\"" + CUrlTemplate::EscapeJson( event.factType.c_str() ).STL_String() +
           "\",\"referenceId\":\"" + event.referenceId + "\",\"author\":\"" + CUrlTemplate::EscapeJson( event.authorId.c_str() ).STL_String() + "\",\"gone\":" + ( event.personNoLongerExists ? "true" : "false" ) + "}";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
