/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CROLLINGFILEACCESS_H
#include "gucefCORE_CRollingFileAccess.h"
#define GUCEF_CORE_CROLLINGFILEACCESS_H
#endif /* GUCEF_CORE_CROLLINGFILEACCESS_H ? */

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

#ifndef GUCEF_CORE_PROCESSARGUMENTS_H
#include "gucefCORE_ProcessArguments.h"
#define GUCEF_CORE_PROCESSARGUMENTS_H
#endif /* GUCEF_CORE_PROCESSARGUMENTS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefLabLink_ConfigFile.h"
#include "gucefLabLink_PlatformDefaults.h"
#include "gucefLabLink_WindowsHost.h"
#include "gucefLabLink_ServiceHost.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The size at which the log file of a Windows service rolls over, and how many old ones are kept */
static const UInt64 LogFileRolloverSize = 10 * 1024 * 1024;
static const Int32 LogFilesKept = 5;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceHost::CServiceHost( bool windowsService )
    : m_windowsService( windowsService )
    , m_arguments()
    , m_fileArguments()
    , m_argv()
    , m_fileArgv()
    , m_problem()
    , m_configFile()
    , m_logFile( GUCEF_NULL )
    , m_logger( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceHost::~CServiceHost()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_logger )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( m_logger );
    }
    GUCEF_DELETE m_logger;
    m_logger = GUCEF_NULL;
    GUCEF_DELETE m_logFile;
    m_logFile = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CServiceHost::TakeConfigOption( const CORE::CString::StringVector& commandLine ,
                                CORE::CString::StringVector& options          ,
                                CORE::CString& configFile                     ,
                                CORE::CString& problem                        )
{GUCEF_TRACE;

    options.clear();
    configFile.Clear();
    for ( size_t i=0; i<commandLine.size(); ++i )
    {
        const CORE::CString& argument = commandLine[ i ];
        if ( argument == "--config" )
        {
            if ( i+1 >= commandLine.size() || commandLine[ i+1 ].IsNULLOrEmpty() )
            {
                problem = "--config needs a file";
                return false;
            }
            configFile = commandLine[ ++i ];
        }
        else
        if ( argument.StartsWith( "--config=" ) )
        {
            configFile = argument.CutChars( 9, true );
            if ( configFile.IsNULLOrEmpty() )
            {
                problem = "--config needs a file";
                return false;
            }
        }
        else
        {
            options.push_back( argument );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CServiceHost::MergeArguments( const CORE::CString& program                      ,
                              const CORE::CString::StringVector& fileOptions    ,
                              const CORE::CString::StringVector& commandOptions )
{GUCEF_TRACE;

    CORE::CString::StringVector arguments;
    arguments.push_back( program );
    arguments.insert( arguments.end(), fileOptions.begin(), fileOptions.end() );
    arguments.insert( arguments.end(), commandOptions.begin(), commandOptions.end() );
    return arguments;
}

/*-------------------------------------------------------------------------*/

void
CServiceHost::MakeArgv( const CORE::CString::StringVector& arguments, GUCEF::vector< char* >& argv )
{GUCEF_TRACE;

    // the strings stay in the vector they belong to, which is not changed afterwards
    argv.clear();
    for ( size_t i=0; i<arguments.size(); ++i )
        argv.push_back( const_cast< char* >( arguments[ i ].C_String() ) );
    argv.push_back( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CServiceHost::Prepare( int argc, char** argv )
{GUCEF_TRACE;

    m_problem.Clear();
    m_configFile.Clear();
    CORE::CString program = ( argc > 0 && GUCEF_NULL != argv && GUCEF_NULL != argv[ 0 ] ) ? CORE::CString( argv[ 0 ] ) : CORE::CString( "gucefLabLink" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    program = "gucefLabLink";       // argv[ 0 ] of the Windows entry point is the argument string
    #endif

    CORE::CString::StringVector processArguments;
    CORE::CString::StringVector commandOptions;
    CORE::CString::StringVector fileOptions;
    CORE::CString configFile;
    bool valid = CORE::GetProcessArguments( argc, argv, processArguments );
    if ( !valid )
        m_problem = "the command line cannot be read";
    valid = valid && TakeConfigOption( processArguments, commandOptions, configFile, m_problem );

    const bool configGiven = !configFile.IsNULLOrEmpty();
    if ( valid && !configGiven )
        configFile = ThisPlatformPaths().configFile;
    if ( valid && !configFile.IsNULLOrEmpty() )
    {
        bool missing = false;
        CORE::CString problem;
        if ( ReadConfigFile( configFile, fileOptions, missing, problem ) )
        {
            m_configFile = configFile;
        }
        else
        if ( configGiven || !missing )
        {
            // a file of the platform that is not there is no problem; one that is refused is
            m_problem = problem;
            valid = false;
        }
    }

    m_arguments = MergeArguments( program, fileOptions, commandOptions );
    m_fileArguments = MergeArguments( program, fileOptions, CORE::CString::StringVector() );
    MakeArgv( m_arguments, m_argv );
    MakeArgv( m_fileArguments, m_fileArgv );
    return valid;
}

/*-------------------------------------------------------------------------*/

int
CServiceHost::GetArgc( void ) const
{GUCEF_TRACE;

    return (int) m_arguments.size();
}

/*-------------------------------------------------------------------------*/

char**
CServiceHost::GetArgv( void )
{GUCEF_TRACE;

    return &m_argv[ 0 ];
}

/*-------------------------------------------------------------------------*/

int
CServiceHost::GetFileArgc( void ) const
{GUCEF_TRACE;

    return (int) m_fileArguments.size();
}

/*-------------------------------------------------------------------------*/

char**
CServiceHost::GetFileArgv( void )
{GUCEF_TRACE;

    return &m_fileArgv[ 0 ];
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceHost::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceHost::GetConfigFile( void ) const
{GUCEF_TRACE;

    return m_configFile;
}

/*-------------------------------------------------------------------------*/

bool
CServiceHost::UsesConsoleLog( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    // the console logger of Windows makes a console when there is none: not for a service, and not when switched off
    return !m_windowsService && CORE::GetEnv( "GUCEF_CONSOLE_LOGGER" ) != "none";
    #else
    return true;
    #endif
}

/*-------------------------------------------------------------------------*/

void
CServiceHost::StartLogging( const CORE::CString& stateDir )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( !m_windowsService )
    {
        AttachToConsole();
        return;
    }
    if ( GUCEF_NULL != m_logger )
        return;

    // the state directory belongs to SYSTEM and the Administrators; the log directory gets that from it
    CORE::CString problem;
    if ( !ProtectStateDirectory( stateDir, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: " << problem );
    }
    const CORE::CString logDir = LogDirFor( stateDir );
    if ( !CORE::DirExists( logDir ) )
        CORE::CreateDirs( logDir );
    const CORE::CString logFile = CORE::CombinePath( logDir, "gucefLabLink.log" );
    m_logFile = GUCEF_NEW CORE::CRollingFileAccess( logFile, "w" );
    m_logFile->SetRolloverSizeThreshold( LogFileRolloverSize );
    m_logFile->SetMaxRolloverFilesBeforeDeletion( LogFilesKept );
    m_logger = GUCEF_NEW CORE::CStdLogger( *m_logFile );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( m_logger );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Runs as a Windows service, the log is " << logFile );

    #endif
}

/*-------------------------------------------------------------------------*/

void
CServiceHost::PrintUsage( void )
{GUCEF_TRACE;

    fprintf( stdout, "Configuration: --config FILE  the options of the command line, one per line (\"name = value\" or \"name\"), read before\n"
                     "  the command line; the command line wins. Default on Windows: %%ProgramData%%\\gucef-lablink\\lablink.conf when it exists.\n"
                     "Windows: runs as the service gucefLabLink (projects/k3s/lablink/install.ps1); --console runs it, and the commands, in a console.\n" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
