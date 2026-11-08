/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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

#ifndef GUCEF_MCPLOCALLLM_PROCESSRUN_H
#define GUCEF_MCPLOCALLLM_PROCESSRUN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A child process whose output is collected. Several can be pumped from one thread in turn.
 *
 *  The standard input of the child is closed straight away: in stdio mode the standard input of this server is the
 *  protocol stream, which a child must never read.
 */
class CProcessRun
{
    public:

    CProcessRun( void );
    ~CProcessRun();

    bool Start( const GUCEF::CORE::CString& exe                    ,
                const GUCEF::CORE::CProcess::TStringVector& args   ,
                const GUCEF::CORE::CString& workingDir             ,
                const GUCEF::CORE::CProcess::TStringMap& env       ,
                GUCEF::UInt32 timeoutInMs                          );

    /**
     *  Reads what the child wrote, waiting up to 'waitInMs' for it. Stops the child at its timeout.
     *  @return true while the child runs
     */
    bool Pump( GUCEF::UInt32 waitInMs );

    /** Stops the child: asks first, then forces it */
    void Stop( bool cancelled );

    /** Runs to the end; for short commands that need no progress or cancellation */
    void RunToEnd( void );

    bool WasStarted( void ) const;
    bool IsFinished( void ) const;
    bool TimedOut( void ) const;
    bool WasCancelled( void ) const;
    GUCEF::Int32 GetExitCode( void ) const;
    GUCEF::UInt64 GetElapsedMs( void ) const;

    /** Standard output and standard error, as they arrived */
    const std::string& GetOutput( void ) const;

    private:

    CProcessRun( const CProcessRun& src );
    CProcessRun& operator=( const CProcessRun& src );

    void Finish( void );

    GUCEF::CORE::CProcess m_process;
    GUCEF::CORE::CString m_exe;
    std::string m_output;
    bool m_started;
    bool m_finished;
    bool m_timedOut;
    bool m_cancelled;
    bool m_open[ 2 ];
    GUCEF::Int32 m_exitCode;
    GUCEF::UInt64 m_startTicks;
    GUCEF::UInt64 m_elapsedMs;
    GUCEF::UInt32 m_timeoutInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Milliseconds since a PrecisionTickCount() */
GUCEF::UInt64 MilliSecsSince( GUCEF::UInt64 startTicks );

/** Output of a git command in 'repoDir'. @return false when it failed, for example because it is not a repository */
bool RunGit( const GUCEF::CORE::CString& repoDir                    ,
             const GUCEF::CORE::CProcess::TStringVector& gitArgs    ,
             GUCEF::CORE::CString& output                           );

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPLOCALLLM_PROCESSRUN_H ? */
