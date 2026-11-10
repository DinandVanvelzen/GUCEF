/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "notifysinkpluginDESKTOP_CNotifySendLauncher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Whatever the sink allows, this many programs at the most are kept track of */
static const UInt32 MaxTrackedChildren = 64;

/** Where the program is looked for when the process has no PATH */
static const char* DefaultSearchPath = "/usr/local/bin:/usr/bin:/bin";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString&
CNotifySendLauncher::GetToolName( void )
{GUCEF_TRACE;

    static const CORE::CString toolName = "notify-send";
    return toolName;
}

/*-------------------------------------------------------------------------*/

bool
CNotifySendLauncher::FindExecutable( const CORE::CString& name, const CORE::CString& searchPath, CORE::CString& foundPath )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || name.HasChar( '/' ) >= 0 || "." == name || ".." == name )
        return false;

    // Only directories given with their full path count: an empty entry or a relative one would make the answer depend on the working directory
    CORE::CString::StringVector directories = searchPath.ParseElements( ':', false );
    for ( size_t i=0; i<directories.size(); ++i )
    {
        const CORE::CString& directory = directories[ i ];
        if ( directory.IsNULLOrEmpty() || '/' != directory.C_String()[ 0 ] )
            continue;

        CORE::CString candidate = directory;
        if ( '/' != directory.C_String()[ directory.ByteSize() - 2 ] )
            candidate += '/';
        candidate += name;

        struct stat info;
        if ( 0 == ::stat( candidate.C_String(), &info ) && S_ISREG( info.st_mode ) && 0 == ::access( candidate.C_String(), X_OK ) )
        {
            foundPath = candidate;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CNotifySendLauncher::CNotifySendLauncher( void )
    : INotifyLauncher()
    , m_searchPath()
    , m_maxRunTimeMs( DefaultMaxRunTimeMs )
    , m_children()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNotifySendLauncher::~CNotifySendLauncher()
{GUCEF_TRACE;

    // The programs were started detached: they are left to finish, we only let go of them
    for ( size_t i=0; i<m_children.size(); ++i )
        GUCEF_DELETE m_children[ i ].process;
    m_children.clear();
}

/*-------------------------------------------------------------------------*/

void
CNotifySendLauncher::SetSearchPath( const CORE::CString& searchPath )
{GUCEF_TRACE;

    m_searchPath = searchPath;
}

/*-------------------------------------------------------------------------*/

void
CNotifySendLauncher::SetMaxRunTimeMs( UInt32 maxRunTimeMs )
{GUCEF_TRACE;

    m_maxRunTimeMs = maxRunTimeMs;
}

/*-------------------------------------------------------------------------*/

bool
CNotifySendLauncher::IsAvailable( void )
{GUCEF_TRACE;

    CORE::CString searchPath = m_searchPath;
    if ( searchPath.IsNULLOrEmpty() )
    {
        const char* environmentPath = ::getenv( "PATH" );
        searchPath = ( GUCEF_NULL != environmentPath ) ? environmentPath : DefaultSearchPath;
    }
    CORE::CString foundPath;
    return FindExecutable( GetToolName(), searchPath, foundPath );
}

/*-------------------------------------------------------------------------*/

void
CNotifySendLauncher::Reap( void )
{GUCEF_TRACE;

    UInt64 now = MT::PrecisionTickCount();
    size_t i = 0;
    while ( i < m_children.size() )
    {
        SChild& child = m_children[ i ];
        bool gone = !child.process->IsRunning();
        if ( !gone && MT::PrecisionTimerTicksToMs( now - child.startTick ) > (Float64) m_maxRunTimeMs )
        {
            // A notification daemon that does not answer must not hold on to a place for ever
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NotifySendLauncher: A notify-send that has been running for more than " << m_maxRunTimeMs << " ms is ended" );
            gone = child.process->Terminate( false );
        }

        if ( gone )
        {
            GUCEF_DELETE child.process;
            m_children.erase( m_children.begin() + i );
        }
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CNotifySendLauncher::Launch( const TArgumentVector& argv )
{GUCEF_TRACE;

    // Only notify-send is ever started, and only the one that was found: what the sink asks for is never taken as a program to run
    if ( argv.empty() || !( GetToolName() == argv[ 0 ] ) )
        return false;

    Reap();
    if ( m_children.size() >= MaxTrackedChildren )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NotifySendLauncher: Too many notify-send programs are running, not starting another" );
        return false;
    }

    CORE::CString searchPath = m_searchPath;
    if ( searchPath.IsNULLOrEmpty() )
    {
        const char* environmentPath = ::getenv( "PATH" );
        searchPath = ( GUCEF_NULL != environmentPath ) ? environmentPath : DefaultSearchPath;
    }
    CORE::CString programPath;
    if ( !FindExecutable( GetToolName(), searchPath, programPath ) )
        return false;

    CORE::CProcess::TStringVector arguments;
    for ( size_t i=1; i<argv.size(); ++i )
        arguments.push_back( argv[ i ] );

    // Own session, standard streams on /dev/null and none of our other descriptors: nothing of the player's terminal or of our sockets is within reach
    CORE::CProcess* process = GUCEF_NEW CORE::CProcess();
    process->SetIsolateFromParent( true );
    if ( !process->Launch( programPath, arguments, CORE::CString(), CORE::CProcess::TStringMap(), true ) )
    {
        GUCEF_DELETE process;
        return false;
    }

    SChild child;
    child.process = process;
    child.startTick = MT::PrecisionTickCount();
    m_children.push_back( child );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CNotifySendLauncher::GetRunningCount( void )
{GUCEF_TRACE;

    Reap();
    return (UInt32) m_children.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
