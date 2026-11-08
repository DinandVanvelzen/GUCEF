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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpLocalLlm_ProcessRun.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt64
MilliSecsSince( UInt64 startTicks )
{
    const UInt64 resolution = MT::PrecisionTimerResolution();
    return resolution > 0 ? ( ( MT::PrecisionTickCount() - startTicks ) * 1000 ) / resolution : 0;
}

/*-------------------------------------------------------------------------*/

bool
RunGit( const CORE::CString& repoDir, const CORE::CProcess::TStringVector& gitArgs, CORE::CString& output )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( "-C" );
    args.push_back( repoDir );
    args.insert( args.end(), gitArgs.begin(), gitArgs.end() );

    CProcessRun run;
    if ( !run.Start( "git", args, CORE::CString::Empty, CORE::CProcess::TStringMap(), 60000 ) )
        return false;
    run.RunToEnd();
    output = run.GetOutput();
    return !run.TimedOut() && 0 == run.GetExitCode();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CProcessRun::CProcessRun( void )
    : m_process()
    , m_exe()
    , m_output()
    , m_started( false )
    , m_finished( false )
    , m_timedOut( false )
    , m_cancelled( false )
    , m_exitCode( -1 )
    , m_startTicks( 0 )
    , m_elapsedMs( 0 )
    , m_timeoutInMs( 0 )
{
    m_open[ 0 ] = m_open[ 1 ] = false;
}

/*-------------------------------------------------------------------------*/

CProcessRun::~CProcessRun()
{
    if ( m_started && !m_finished )
        Stop( true );
}

/*-------------------------------------------------------------------------*/

bool
CProcessRun::Start( const CORE::CString& exe                    ,
                    const CORE::CProcess::TStringVector& args   ,
                    const CORE::CString& workingDir             ,
                    const CORE::CProcess::TStringMap& env       ,
                    UInt32 timeoutInMs                          )
{GUCEF_TRACE;

    m_exe = exe;
    m_timeoutInMs = timeoutInMs;
    m_startTicks = MT::PrecisionTickCount();
    m_started = m_process.LaunchWithPipes( exe, args, true, true, true, workingDir, env );
    if ( !m_started )
    {
        m_finished = true;
        return false;
    }
    m_process.CloseStdinPipe();
    m_open[ 0 ] = GUCEF_NULL != m_process.GetStdoutPipe();
    m_open[ 1 ] = GUCEF_NULL != m_process.GetStderrPipe();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProcessRun::Pump( UInt32 waitInMs )
{
    if ( !m_started || m_finished )
        return false;

    CORE::CProcessPipe* pipes[ 2 ] = { m_process.GetStdoutPipe(), m_process.GetStderrPipe() };
    char buffer[ 8192 ];
    bool waited = false;
    for ( int i=0; i<2; ++i )
    {
        if ( !m_open[ i ] )
            continue;
        // the wait goes to the first pipe that is open; the other is only checked
        const UInt32 wait = waited ? 0 : waitInMs;
        waited = true;
        if ( pipes[ i ]->WaitForData( wait ) )
        {
            const UInt32 bytesRead = pipes[ i ]->ReadSome( buffer, sizeof( buffer ) );
            if ( 0 == bytesRead )
                m_open[ i ] = false;
            else
                m_output.append( buffer, bytesRead );
        }
    }

    if ( !m_open[ 0 ] && !m_open[ 1 ] )
    {
        Finish();
        return false;
    }
    if ( MilliSecsSince( m_startTicks ) > m_timeoutInMs )
    {
        m_timedOut = true;
        Stop( false );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CProcessRun::Finish( void )
{
    if ( !m_process.WaitForExit( 10000, m_exitCode ) )
    {
        m_process.Terminate( false, 0 );
        m_exitCode = -1;
    }
    m_elapsedMs = MilliSecsSince( m_startTicks );
    m_finished = true;
}

/*-------------------------------------------------------------------------*/

void
CProcessRun::Stop( bool cancelled )
{GUCEF_TRACE;

    if ( !m_started || m_finished )
        return;
    m_cancelled = cancelled;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "gucefMcpLocalLlm: Stopping " ) + m_exe + ( cancelled ? " (cancelled)" : " (timeout)" ) );
    m_process.Terminate( true, 5000 );
    Finish();
}

/*-------------------------------------------------------------------------*/

void
CProcessRun::RunToEnd( void )
{
    while ( Pump( 100 ) ) {}
}

/*-------------------------------------------------------------------------*/

bool CProcessRun::WasStarted( void ) const { return m_started; }
bool CProcessRun::IsFinished( void ) const { return m_finished; }
bool CProcessRun::TimedOut( void ) const { return m_timedOut; }
bool CProcessRun::WasCancelled( void ) const { return m_cancelled; }
Int32 CProcessRun::GetExitCode( void ) const { return m_exitCode; }
UInt64 CProcessRun::GetElapsedMs( void ) const { return m_finished ? m_elapsedMs : MilliSecsSince( m_startTicks ); }
const std::string& CProcessRun::GetOutput( void ) const { return m_output; }

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/
