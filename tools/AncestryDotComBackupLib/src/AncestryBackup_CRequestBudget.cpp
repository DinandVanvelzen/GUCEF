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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <ctime>
#include <cstdlib>
#include <cstdio>

#include "AncestryBackup_CRequestBudget.h"

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

CRequestBudget::CRequestBudget( void )
    : m_delayInMs( 2500 )
    , m_jitterPercent( 40 )
    , m_maxPerRun( 500 )
    , m_maxPerDay( 3000 )
    , m_stateFile()
    , m_day()
    , m_todayCount( 0 )
    , m_runCount( 0 )
    , m_lastRequestInMs( 0 )
    , m_nextDelayInMs( 0 )
    , m_nextDelayChosen( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRequestBudget::Configure( UInt32 delayInMs, UInt32 jitterPercent, UInt32 maxPerRun, UInt32 maxPerDay )
{GUCEF_TRACE;

    m_delayInMs = delayInMs;
    m_jitterPercent = jitterPercent > 100 ? 100 : jitterPercent;
    m_maxPerRun = maxPerRun;
    m_maxPerDay = maxPerDay;
    m_nextDelayChosen = false;
}

/*-------------------------------------------------------------------------*/

std::string
CRequestBudget::DayOf( UInt64 unixEpochInMs )
{GUCEF_TRACE;

    const time_t seconds = (time_t) ( unixEpochInMs / 1000 );
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    gmtime_s( &parts, &seconds );
    #else
    gmtime_r( &seconds, &parts );
    #endif
    char text[ 16 ];
    strftime( text, sizeof( text ), "%Y%m%d", &parts );
    return text;
}

/*-------------------------------------------------------------------------*/

void
CRequestBudget::SetStateFile( const CORE::CString& path, UInt64 nowInMs )
{GUCEF_TRACE;

    m_stateFile = path;
    m_day = DayOf( nowInMs );
    m_todayCount = 0;

    if ( !CORE::FileExists( path ) )
        return;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return;
    std::string content( (size_t) file.GetSize(), '\0' );
    if ( !content.empty() && file.Read( &content[ 0 ], 1, (UInt32) content.size() ) != content.size() )
        return;

    // day=20261017\ncount=12
    const size_t d = content.find( "day=" );
    const size_t c = content.find( "count=" );
    if ( d != std::string::npos && c != std::string::npos && content.compare( d + 4, 8, m_day ) == 0 )
        m_todayCount = (UInt32) strtoul( content.c_str() + c + 6, GUCEF_NULL, 10 );
}

/*-------------------------------------------------------------------------*/

void
CRequestBudget::Save( void ) const
{GUCEF_TRACE;

    if ( m_stateFile.IsNULLOrEmpty() )
        return;
    CORE::WriteStringAsTextFile( m_stateFile, CORE::CString( ( "day=" + m_day + "\ncount=" + std::to_string( (unsigned long long) m_todayCount ) + "\n" ).c_str() ) );
}

/*-------------------------------------------------------------------------*/

bool
CRequestBudget::CanRequest( UInt64 nowInMs, std::string& reason )
{GUCEF_TRACE;

    // the day may have turned since the count was read
    const std::string today = DayOf( nowInMs );
    if ( !m_day.empty() && today != m_day )
    {
        m_day = today;
        m_todayCount = 0;
    }

    if ( m_maxPerRun > 0 && m_runCount >= m_maxPerRun )
    {
        reason = "the maximum of " + std::to_string( (unsigned long long) m_maxPerRun ) + " requests for one run was reached";
        return false;
    }
    if ( m_maxPerDay > 0 && m_todayCount >= m_maxPerDay )
    {
        reason = "the maximum of " + std::to_string( (unsigned long long) m_maxPerDay ) + " requests for one day was reached";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRequestBudget::GetDelayBeforeNextInMs( UInt64 nowInMs ) const
{GUCEF_TRACE;

    if ( m_lastRequestInMs == 0 )
        return 0;

    if ( !m_nextDelayChosen )
    {
        // between (100 - jitter)% and (100 + jitter)% of the delay
        const UInt64 spread = (UInt64) m_delayInMs * m_jitterPercent / 100;
        const UInt32 low = (UInt32) ( m_delayInMs - spread );
        const UInt32 high = (UInt32) ( m_delayInMs + spread );
        m_nextDelayInMs = high > low ? CORE::CRandom::GetRandomUInt32( low, high ) : low;
        m_nextDelayChosen = true;
    }
    const UInt64 due = m_lastRequestInMs + m_nextDelayInMs;
    return nowInMs >= due ? 0 : (UInt32) ( due - nowInMs );
}

/*-------------------------------------------------------------------------*/

void
CRequestBudget::RecordRequest( UInt64 nowInMs )
{GUCEF_TRACE;

    ++m_runCount;
    ++m_todayCount;
    m_lastRequestInMs = nowInMs;
    m_nextDelayChosen = false;
    Save();
}

/*-------------------------------------------------------------------------*/

UInt32
CRequestBudget::GetRunCount( void ) const
{GUCEF_TRACE;

    return m_runCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CRequestBudget::GetTodayCount( void ) const
{GUCEF_TRACE;

    return m_todayCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHaltState::GetPath( const CORE::CString& stateDir )
{GUCEF_TRACE;

    return CORE::CombinePath( stateDir, "HALTED.txt" );
}

/*-------------------------------------------------------------------------*/

bool
CHaltState::IsHalted( const CORE::CString& stateDir, std::string& reason )
{GUCEF_TRACE;

    const CORE::CString path = GetPath( stateDir );
    if ( !CORE::FileExists( path ) )
        return false;
    CORE::CFileAccess file( path, "rb" );
    reason.clear();
    if ( file.Opened() )
    {
        reason.resize( (size_t) file.GetSize() );
        if ( !reason.empty() && file.Read( &reason[ 0 ], 1, (UInt32) reason.size() ) != reason.size() )
            reason.clear();
    }
    if ( reason.empty() )
        reason = "(no reason was written down)";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHaltState::Halt( const CORE::CString& stateDir, const std::string& reason )
{GUCEF_TRACE;

    return CORE::CreateDirs( stateDir ) && CORE::WriteStringAsTextFile( GetPath( stateDir ), CORE::CString( reason.c_str() ) );
}

/*-------------------------------------------------------------------------*/

void
CHaltState::Clear( const CORE::CString& stateDir )
{GUCEF_TRACE;

    CORE::DeleteFile( GetPath( stateDir ) );
}

/*-------------------------------------------------------------------------*/

static std::string
LowerOf( const std::string& text )
{
    std::string lower = text;
    for ( size_t i=0; i<lower.size(); ++i )
        lower[ i ] = (char) tolower( lower[ i ] );
    return lower;
}

/*-------------------------------------------------------------------------*/

bool
CSiteResponseCheck::IsTrouble( Int32 status, const std::string& contentType, const std::string& mitigationHeader, const std::string& bodyStart, bool expectJson, std::string& reason )
{GUCEF_TRACE;

    if ( status == 429 )
    {
        reason = "the site answered 429, too many requests";
        return true;
    }
    if ( status == 403 || status == 401 )
    {
        reason = "the site answered " + std::to_string( (long long) status ) + " (refused)";
        return true;
    }
    if ( status == 503 || status == 502 || status == 504 )
    {
        reason = "the site answered " + std::to_string( (long long) status ) + " (unavailable or overloaded)";
        return true;
    }
    if ( !mitigationHeader.empty() )
    {
        reason = "the site served a challenge (cf-mitigated: " + mitigationHeader + ")";
        return true;
    }

    const std::string type = LowerOf( contentType );
    const std::string start = LowerOf( bodyStart.substr( 0, 4096 ) );
    static const char* const markers[] = { "just a moment", "captcha", "unusual traffic", "verify you are human", "checking your browser", "access denied", "challenge-platform", GUCEF_NULL };
    if ( type.find( "html" ) != std::string::npos )
    {
        for ( UInt32 i=0; markers[ i ] != GUCEF_NULL; ++i )
            if ( start.find( markers[ i ] ) != std::string::npos )
            {
                reason = std::string( "the site served a page that looks like a challenge (" ) + markers[ i ] + ")";
                return true;
            }
        if ( expectJson )
        {
            reason = "an html page came back where JSON was asked for";
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
