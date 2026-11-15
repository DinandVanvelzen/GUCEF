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

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include "gucefCORE_CServiceResourceWatch.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

const UInt32 CServiceResourceWatch::DefaultSampleMs;
const UInt32 CServiceResourceWatch::DefaultLogSeconds;
const UInt32 CServiceResourceWatch::ReleasePercent;

/*-------------------------------------------------------------------------*/

namespace {

const UInt64 BytesPerMB = 1024 * 1024;

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

/** The value of a "Name:   1234 kB" line of /proc/self/status, in bytes for kB values */
bool
ReadStatusValue( const char* line, const char* name, UInt64& value, bool kiloBytes )
{GUCEF_TRACE;

    const size_t nameLength = strlen( name );
    if ( 0 != strncmp( line, name, nameLength ) || ':' != line[ nameLength ] )
        return false;
    const char* cursor = line + nameLength + 1;
    while ( ' ' == *cursor || '\t' == *cursor )
        ++cursor;
    char* end = GUCEF_NULL;
    const unsigned long long number = strtoull( cursor, &end, 10 );
    if ( end == cursor )
        return false;
    value = (UInt64) number;
    if ( kiloBytes )
        value *= 1024;
    return true;
}

#endif

}

/*-------------------------------------------------------------------------*/

SServiceProcessResources::SServiceProcessResources( void )
    : sampledAtMs( 0 )
    , rssBytes( 0 )
    , hwmBytes( 0 )
    , threadCount( 0 )
    , openDescriptors( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceResourceWatch::CServiceResourceWatch( void )
    : m_lock()
    , m_sources()
    , m_sampleMs( DefaultSampleMs )
    , m_logSeconds( DefaultLogSeconds )
    , m_softLimitBytes( 0 )
    , m_aboveSoftLimit( false )
    , m_lastSampleMs( 0 )
    , m_lastLogMs( 0 )
    , m_lastProcess()
    , m_lastCounters()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceResourceWatch::~CServiceResourceWatch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CServiceResourceWatch::Configure( UInt32 sampleMs, UInt32 logSeconds, UInt64 memorySoftLimitBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_sampleMs = 0 == sampleMs ? DefaultSampleMs : sampleMs;
    m_logSeconds = logSeconds;
    m_softLimitBytes = memorySoftLimitBytes;
    if ( 0 == m_softLimitBytes )
        m_aboveSoftLimit = false;
}

/*-------------------------------------------------------------------------*/

void
CServiceResourceWatch::AddSource( CIServiceResourceSource* source )
{GUCEF_TRACE;

    if ( GUCEF_NULL == source )
        return;
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_sources.size(); ++i )
    {
        if ( m_sources[ i ] == source )
            return;
    }
    m_sources.push_back( source );
}

/*-------------------------------------------------------------------------*/

void
CServiceResourceWatch::RemoveSource( CIServiceResourceSource* source )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TSourceVector::iterator i = m_sources.begin();
    while ( i != m_sources.end() )
    {
        if ( (*i) == source )
            i = m_sources.erase( i );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CServiceResourceWatch::Tick( UInt64 nowMs )
{GUCEF_TRACE;

    bool sample = false;
    bool log = false;
    {
        MT::CScopeMutex lock( m_lock );
        sample = 0 == m_lastSampleMs || nowMs >= m_lastSampleMs + m_sampleMs;
        log = 0 != m_logSeconds && ( 0 == m_lastLogMs || nowMs >= m_lastLogMs + (UInt64) m_logSeconds * 1000 );
    }
    if ( sample || log )
        SampleNow( nowMs, log );
}

/*-------------------------------------------------------------------------*/

bool
CServiceResourceWatch::SampleProcess( SServiceProcessResources& resources )
{GUCEF_TRACE;

    return ReadProcessResources( resources );
}

/*-------------------------------------------------------------------------*/

void
CServiceResourceWatch::SampleNow( UInt64 nowMs, bool log )
{GUCEF_TRACE;

    SServiceProcessResources process;
    SampleProcess( process );
    process.sampledAtMs = 0 == nowMs ? 1 : nowMs;

    // the counters are read outside the lock: a source may take a lock of its own
    TSourceVector sources;
    {
        MT::CScopeMutex lock( m_lock );
        sources = m_sources;
    }
    CValueList counters;
    for ( UInt32 i=0; i<sources.size(); ++i )
    {
        CValueList sourceCounters;
        sources[ i ]->GetResourceCounters( sourceCounters );
        const CString prefix = sources[ i ]->GetResourceSourceName() + ".";
        for ( UInt32 k=0; k<sourceCounters.GetKeyCount(); ++k )
            counters.Set( prefix + sourceCounters.GetKey( k ).AsString(), sourceCounters.GetValue( k ) );
    }

    bool wentAbove = false;
    bool wentBelow = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_lastProcess = process;
        m_lastCounters = counters;
        m_lastSampleMs = process.sampledAtMs;
        if ( 0 != m_softLimitBytes && 0 != process.rssBytes )
        {
            if ( !m_aboveSoftLimit && process.rssBytes > m_softLimitBytes )
            {
                m_aboveSoftLimit = true;
                wentAbove = true;
            }
            else
            if ( m_aboveSoftLimit && process.rssBytes < ( m_softLimitBytes / 100 ) * ReleasePercent )
            {
                m_aboveSoftLimit = false;
                wentBelow = true;
            }
        }
        if ( log )
            m_lastLogMs = process.sampledAtMs;
    }

    if ( wentAbove )
        GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "ResourceWatch: the resident memory of " << ( process.rssBytes / BytesPerMB )
                           << " MB is above the soft limit of " << ( m_softLimitBytes / BytesPerMB ) << " MB" );
    if ( wentBelow )
        GUCEF_LOG( LOGLEVEL_NORMAL, "ResourceWatch: the resident memory of " << ( process.rssBytes / BytesPerMB )
                   << " MB is below " << ReleasePercent << "% of the soft limit again" );
    if ( log )
        GUCEF_LOG( LOGLEVEL_NORMAL, FormatLine() );
}

/*-------------------------------------------------------------------------*/

SServiceProcessResources
CServiceResourceWatch::GetLastProcessSample( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastProcess;
}

/*-------------------------------------------------------------------------*/

CValueList
CServiceResourceWatch::GetLastCounters( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastCounters;
}

/*-------------------------------------------------------------------------*/

bool
CServiceResourceWatch::IsAboveSoftLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_aboveSoftLimit;
}

/*-------------------------------------------------------------------------*/

void
CServiceResourceWatch::GetInfo( CValueList& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    info.Set( "rssMB", CVariant( m_lastProcess.rssBytes / BytesPerMB ) );
    info.Set( "hwmMB", CVariant( m_lastProcess.hwmBytes / BytesPerMB ) );
    info.Set( "threads", CVariant( m_lastProcess.threadCount ) );
    info.Set( "descriptors", CVariant( m_lastProcess.openDescriptors ) );
    info.Set( "memorySoftLimitMB", CVariant( m_softLimitBytes / BytesPerMB ) );
    info.Set( "aboveSoftLimit", CVariant( m_aboveSoftLimit ) );
    for ( UInt32 k=0; k<m_lastCounters.GetKeyCount(); ++k )
        info.Set( m_lastCounters.GetKey( k ), m_lastCounters.GetValue( k ) );
}

/*-------------------------------------------------------------------------*/

CString
CServiceResourceWatch::FormatLine( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CString line = "ResourceWatch: rssMB=" + ToString( m_lastProcess.rssBytes / BytesPerMB ) +
                   " hwmMB=" + ToString( m_lastProcess.hwmBytes / BytesPerMB ) +
                   " threads=" + ToString( m_lastProcess.threadCount ) +
                   " fds=" + ToString( m_lastProcess.openDescriptors );
    if ( m_aboveSoftLimit )
        line += " aboveSoftLimit=true";
    for ( UInt32 k=0; k<m_lastCounters.GetKeyCount(); ++k )
        line += " " + m_lastCounters.GetKey( k ).AsString() + "=" + m_lastCounters.GetValue( k ).AsString();
    return line;
}

/*-------------------------------------------------------------------------*/

bool
CServiceResourceWatch::ReadProcessResources( SServiceProcessResources& resources )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    // /proc files report no size, so they are read with stdio until their end
    FILE* status = fopen( "/proc/self/status", "r" );
    if ( GUCEF_NULL == status )
        return false;
    char line[ 256 ];
    UInt64 value = 0;
    while ( GUCEF_NULL != fgets( line, sizeof( line ), status ) )
    {
        if ( ReadStatusValue( line, "VmRSS", value, true ) )
            resources.rssBytes = value;
        else
        if ( ReadStatusValue( line, "VmHWM", value, true ) )
            resources.hwmBytes = value;
        else
        if ( ReadStatusValue( line, "Threads", value, false ) )
            resources.threadCount = (UInt32) value;
    }
    fclose( status );

    UInt32 descriptors = 0;
    CFileSystemIterator entries;
    if ( entries.FindFirst( "/proc/self/fd" ) )
    {
        do
        {
            const CString name = entries.GetResourceName();
            if ( name != "." && name != ".." )
                ++descriptors;
        }
        while ( entries.FindNext() );
        entries.FindClose();
    }
    resources.openDescriptors = descriptors;
    return 0 != resources.rssBytes;

    #else

    // other platforms say nothing yet
    return false;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
