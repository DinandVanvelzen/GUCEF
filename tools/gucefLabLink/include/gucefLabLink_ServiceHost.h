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

#ifndef GUCEF_LABLINK_SERVICEHOST_H
#define GUCEF_LABLINK_SERVICEHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace CORE { class CRollingFileAccess; class CStdLogger; }; };

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the program needs from its platform before the options are parsed and while it runs as a service:
 *  - the arguments: those of the process (CORE::GetProcessArguments(): on Windows the command line of the process
 *    split again, also in service mode, where the entry point gives the service start parameters and with --console
 *    the whole command line as one argument), with the options of the configuration file in front of them;
 *  - the configuration file: --config FILE (or --config=FILE) on the command line, else the one of the platform
 *    (Windows: %ProgramData%\gucef-lablink\lablink.conf, when it exists; other platforms: none). A file named with
 *    --config must exist. The command line comes after the file: an option given in both takes the value of the
 *    command line, and repeatable options (--allow) add up;
 *  - the log: a Windows service logs to a rotating file, logs\gucefLabLink.log in the state directory (it has no
 *    console); on Windows --console and the commands get the console of the command prompt.
 *  On Linux, without --config, the arguments are exactly those of main().
 */
class CServiceHost
{
    public:

    /** @param windowsService   whether the program runs as a service of the Windows service control manager */
    explicit CServiceHost( bool windowsService );

    ~CServiceHost();

    /** The arguments and the configuration file; false with GetProblem() when the file is refused */
    bool Prepare( int argc, char** argv );

    /** argv[ 0 ], then the options of the file, then those of the command line without --config: for the option parser */
    int GetArgc( void ) const;
    char** GetArgv( void );

    /** argv[ 0 ] and the options of the configuration file alone, to check that they hold settings only */
    int GetFileArgc( void ) const;
    char** GetFileArgv( void );

    const GUCEF::CORE::CString& GetProblem( void ) const;

    /** The configuration file that was read; empty when none was */
    const GUCEF::CORE::CString& GetConfigFile( void ) const;

    /** Windows: the console for a console run, the log file for a service (in 'stateDir'); elsewhere nothing */
    void StartLogging( const GUCEF::CORE::CString& stateDir );

    /** Whether the console logger is wanted: not for a Windows service, which has no console */
    bool UsesConsoleLog( void ) const;

    /** Usage lines of the options above, after those of the main program */
    static void PrintUsage( void );

    /**
     *  Takes --config FILE and --config=FILE out of 'commandLine' (the arguments without the program); the last one
     *  counts. False when --config has no value.
     */
    static bool TakeConfigOption( const GUCEF::CORE::CString::StringVector& commandLine ,
                                  GUCEF::CORE::CString::StringVector& options          ,
                                  GUCEF::CORE::CString& configFile                     ,
                                  GUCEF::CORE::CString& problem                        );

    /** 'program', the options of the file, then those of the command line */
    static GUCEF::CORE::CString::StringVector MergeArguments( const GUCEF::CORE::CString& program                     ,
                                                              const GUCEF::CORE::CString::StringVector& fileOptions    ,
                                                              const GUCEF::CORE::CString::StringVector& commandOptions );

    private:

    CServiceHost( const CServiceHost& src );                /**< not implemented */
    CServiceHost& operator=( const CServiceHost& src );     /**< not implemented */

    static void MakeArgv( const GUCEF::CORE::CString::StringVector& arguments, GUCEF::vector< char* >& argv );

    bool m_windowsService;
    GUCEF::CORE::CString::StringVector m_arguments;
    GUCEF::CORE::CString::StringVector m_fileArguments;
    GUCEF::vector< char* > m_argv;
    GUCEF::vector< char* > m_fileArgv;
    GUCEF::CORE::CString m_problem;
    GUCEF::CORE::CString m_configFile;
    GUCEF::CORE::CRollingFileAccess* m_logFile;
    GUCEF::CORE::CStdLogger* m_logger;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_SERVICEHOST_H ? */
