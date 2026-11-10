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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ANCESTRYBACKUP_CDNAMATCHES_H
#include "AncestryBackup_CDnaMatches.h"
#define ANCESTRYBACKUP_CDNAMATCHES_H
#endif /* ANCESTRYBACKUP_CDNAMATCHES_H ? */

#include "AncestryBackup_CSettings.h"

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

CSettings::CSettings( void )
    : dataDir( "data" )
    , treeId( "default" )
    , dnaSampleId()
    , siteTreeId()
    , chromePath()
    , startUrl( "https://www.ancestry.com/" )
    , cookieDomain( "ancestry.com" )
    , grampsFile()
    , grampsMediaPath()
    , grampsHomePerson()
    , researcherName()
    , researcherEmail()
    , baselineGedcom()
    , exportDirs()
    , exportMaxAgeDays( 7 )
    , changesSince()
    , changesMaxPages( 50 )
    , requestDelayInMs( 2500 )
    , requestJitterPercent( 40 )
    , maxRequestsPerRun( 500 )
    , maxRequestsPerDay( 3000 )
    , debugPort( 9222 )
    , discoverSeconds( 0 )
    , maxBodyBytes( 32ULL * 1024ULL * 1024ULL )
    , headless( false )
    , noWait( false )
    , attachToRunning( true )
    , retryMissing( false )
    , recordDna( false )
    , chromeUserDataDirs()
    , hostFilter()
    , chromeArgs()
{GUCEF_TRACE;

    hostFilter.push_back( "ancestry" );
}

/*-------------------------------------------------------------------------*/

std::vector< std::string >
CSettings::SplitList( const std::string& text )
{GUCEF_TRACE;

    std::vector< std::string > items;
    std::string item;
    for ( size_t i=0; i<=text.size(); ++i )
    {
        if ( i == text.size() || text[ i ] == ';' || text[ i ] == ',' )
        {
            // trim
            size_t b = item.find_first_not_of( " \t" );
            size_t e = item.find_last_not_of( " \t" );
            if ( b != std::string::npos )
                items.push_back( item.substr( b, e - b + 1 ) );
            item.clear();
        }
        else
            item += text[ i ];
    }
    return items;
}

/*-------------------------------------------------------------------------*/

static std::string
Get( const std::map< std::string, std::string >& options, const char* name, bool& found )
{
    std::map< std::string, std::string >::const_iterator i = options.find( name );
    found = i != options.end();
    return found ? i->second : std::string();
}

/*-------------------------------------------------------------------------*/

static bool
ToBool( const std::string& text )
{
    return text.empty() || text == "1" || text == "true" || text == "yes" || text == "on";
}

/*-------------------------------------------------------------------------*/

bool
CSettings::Load( const std::map< std::string, std::string >& options, CORE::CString& problem )
{GUCEF_TRACE;

    bool found = false;
    std::string value;

    value = Get( options, "datadir", found );
    if ( found && !value.empty() ) dataDir = value.c_str();

    value = Get( options, "treeid", found );
    if ( found && !value.empty() ) treeId = value.c_str();

    value = Get( options, "dnasampleid", found );
    if ( found && !value.empty() )
    {
        if ( !CDnaMatches::LooksLikeSampleId( value ) )
        {
            problem = "dnaSampleId has to look like 53F46AE4-480D-4848-8E12-C3CA3666D26C, got \"" + CORE::CString( value.c_str() ) + "\"";
            return false;
        }
        dnaSampleId = value.c_str();
    }

    value = Get( options, "sitetreeid", found );
    if ( found && !value.empty() )
    {
        if ( value.find_first_not_of( "0123456789" ) != std::string::npos )
        {
            problem = "siteTreeId has to be the number Ancestry uses for the tree, got \"" + CORE::CString( value.c_str() ) + "\"";
            return false;
        }
        siteTreeId = value.c_str();
    }

    value = Get( options, "chromepath", found );
    if ( found ) chromePath = value.c_str();

    value = Get( options, "starturl", found );
    if ( found && !value.empty() ) startUrl = value.c_str();

    value = Get( options, "cookiedomain", found );
    if ( found && !value.empty() ) cookieDomain = value.c_str();

    value = Get( options, "debugport", found );
    if ( found )
    {
        const UInt32 port = CORE::StringToUInt32( value.c_str(), 0 );
        if ( port == 0 || port > 65535 )
        {
            problem = "debugPort has to be a port number, got \"" + CORE::CString( value.c_str() ) + "\"";
            return false;
        }
        debugPort = (UInt16) port;
    }

    value = Get( options, "discoverseconds", found );
    if ( found ) discoverSeconds = CORE::StringToUInt32( value.c_str(), 0 );

    value = Get( options, "maxbodymegabytes", found );
    if ( found ) maxBodyBytes = CORE::StringToUInt64( value.c_str(), 32 ) * 1024ULL * 1024ULL;

    value = Get( options, "headless", found );
    if ( found ) headless = ToBool( value );

    value = Get( options, "nowait", found );
    if ( found ) noWait = ToBool( value );

    value = Get( options, "gedcom", found );
    if ( found && !value.empty() ) baselineGedcom = value.c_str();
    value = Get( options, "baselinegedcom", found );
    if ( found && !value.empty() ) baselineGedcom = value.c_str();

    value = Get( options, "grampsfile", found );
    if ( found && !value.empty() ) grampsFile = value.c_str();
    value = Get( options, "grampsmediapath", found );
    if ( found && !value.empty() ) grampsMediaPath = value.c_str();
    value = Get( options, "grampshomeperson", found );
    if ( found && !value.empty() ) grampsHomePerson = value.c_str();
    value = Get( options, "researchername", found );
    if ( found && !value.empty() ) researcherName = value.c_str();
    value = Get( options, "researcheremail", found );
    if ( found && !value.empty() ) researcherEmail = value.c_str();

    value = Get( options, "exportdirs", found );
    if ( found ) exportDirs = SplitList( value );

    value = Get( options, "exportmaxagedays", found );
    if ( found )
    {
        exportMaxAgeDays = CORE::StringToUInt32( value.c_str(), 0 );
        if ( exportMaxAgeDays == 0 )
        {
            problem = "exportMaxAgeDays has to be a number of days, got \"" + CORE::CString( value.c_str() ) + "\"";
            return false;
        }
    }

    value = Get( options, "changessince", found );
    if ( found ) changesSince = value.c_str();
    value = Get( options, "changesmaxpages", found );
    if ( found ) changesMaxPages = CORE::StringToUInt32( value.c_str(), 50 );

    value = Get( options, "requestdelayms", found );
    if ( found )
    {
        requestDelayInMs = CORE::StringToUInt32( value.c_str(), 2500 );
        if ( requestDelayInMs < 500 )
        {
            problem = "requestDelayMs may not be below 500, the tool is not meant to hurry";
            return false;
        }
    }
    value = Get( options, "requestjitterpercent", found );
    if ( found ) requestJitterPercent = CORE::StringToUInt32( value.c_str(), 40 );
    value = Get( options, "maxrequestsperrun", found );
    if ( found ) maxRequestsPerRun = CORE::StringToUInt32( value.c_str(), 500 );
    value = Get( options, "maxrequestsperday", found );
    if ( found ) maxRequestsPerDay = CORE::StringToUInt32( value.c_str(), 3000 );

    value = Get( options, "retrymissing", found );
    if ( found ) retryMissing = ToBool( value );

    value = Get( options, "recorddna", found );
    if ( found ) recordDna = ToBool( value );

    value = Get( options, "attachtorunning", found );
    if ( found ) attachToRunning = ToBool( value );

    value = Get( options, "chromeuserdatadir", found );
    if ( found ) chromeUserDataDirs = SplitList( value );

    value = Get( options, "hostfilter", found );
    if ( found ) hostFilter = SplitList( value );

    value = Get( options, "chromeargs", found );
    if ( found ) chromeArgs = SplitList( value );

    if ( !CDataLayout::IsValidTreeId( treeId ) )
    {
        problem = "treeId may only contain letters, digits, '-', '_' and '.', got \"" + treeId + "\"";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CDataLayout
CSettings::GetLayout( void ) const
{GUCEF_TRACE;

    return CDataLayout( CORE::RelativePath( dataDir ), treeId );
}

/*-------------------------------------------------------------------------*/

bool
CSettings::ParseCommandLine( const std::vector< std::string >& args                  ,
                             std::string& command                                    ,
                             std::map< std::string, std::string >& options           ,
                             CORE::CString& problem                                  )
{GUCEF_TRACE;

    command.clear();
    options.clear();

    for ( size_t i=0; i<args.size(); ++i )
    {
        std::string arg = args[ i ];
        while ( !arg.empty() && arg[ 0 ] == '-' )
            arg.erase( 0, 1 );
        if ( arg.empty() )
            continue;

        const size_t eq = arg.find( '=' );
        if ( eq == std::string::npos && args[ i ][ 0 ] != '-' )
        {
            if ( !command.empty() )
            {
                problem = "More than one command was given: \"" + CORE::CString( command.c_str() ) + "\" and \"" + CORE::CString( arg.c_str() ) + "\"";
                return false;
            }
            command = arg;
            continue;
        }

        std::string name = eq == std::string::npos ? arg : arg.substr( 0, eq );
        for ( size_t c=0; c<name.size(); ++c )
            name[ c ] = (char) tolower( name[ c ] );
        options[ name ] = eq == std::string::npos ? std::string() : arg.substr( eq + 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
