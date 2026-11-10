/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPROCESS_H
#define GUCEF_CORE_CPROCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>
#include <map>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPROCESSPIPE_H
#include "gucefCORE_CProcessPipe.h"
#define GUCEF_CORE_CPROCESSPIPE_H
#endif /* GUCEF_CORE_CPROCESSPIPE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Launches and controls a child process.
 *
 *  An instance manages at most one process at a time. Unless the process was launched detached the
 *  destructor terminates a process that is still running. A detached process is never killed by this class.
 *  Not thread-safe: use an instance from one thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CProcess
{
    public:

    typedef std::vector< CString >          TStringVector;
    typedef std::map< CString, CString >    TStringMap;

    CProcess( void );

    ~CProcess();

    /**
     *  Starts a new process
     *  @param exePath      path of the executable, or just its name in which case the O/S search path is used
     *  @param args         arguments not including argv[0], which is set to exePath
     *  @param workingDir   working directory for the child, empty to inherit ours
     *  @param envOverrides environment variables to set (or replace) in the child on top of our own environment
     *  @param detach       if true the child is started in its own session/process group so it survives us and
     *                      does not receive terminal signals meant for us
     *  @return true if the process was started. Failing to find or execute the exe is reported as failure
     */
    bool Launch( const CString& exePath                  ,
                 const TStringVector& args               ,
                 const CString& workingDir = CString()   ,
                 const TStringMap& envOverrides = TStringMap() ,
                 bool detach = false                     );

    /**
     *  Starts a new process with its standard streams redirected to pipes that this process holds the other end of.
     *  A stream that is not redirected is inherited from this process, unless SetIsolateFromParent() says otherwise.
     *  The pipes are available through GetStdinPipe(), GetStdoutPipe() and GetStderrPipe() until the next launch or the
     *  destruction of this object. A child that fills a pipe nobody reads blocks, so a caller that redirects output has
     *  to read it. Closing the standard input pipe (CloseStdinPipe()) is how a child is told the input has ended.
     *  @param pipeStdin    redirect the standard input of the child: its writing end is GetStdinPipe()
     *  @param pipeStdout   redirect the standard output of the child: its reading end is GetStdoutPipe()
     *  @param pipeStderr   redirect the standard error of the child: its reading end is GetStderrPipe()
     *  @return true if the process was started. Failing to find or execute the exe is reported as failure
     */
    bool LaunchWithPipes( const CString& exePath                  ,
                          const TStringVector& args               ,
                          bool pipeStdin                          ,
                          bool pipeStdout                         ,
                          bool pipeStderr                         ,
                          const CString& workingDir = CString()   ,
                          const TStringMap& envOverrides = TStringMap() );

    /** The pipe to the standard input of the child, GUCEF_NULL when it was not redirected. Owned by this object */
    CProcessPipe* GetStdinPipe( void ) const;

    /** The pipe from the standard output of the child, GUCEF_NULL when it was not redirected. Owned by this object */
    CProcessPipe* GetStdoutPipe( void ) const;

    /** The pipe from the standard error of the child, GUCEF_NULL when it was not redirected. Owned by this object */
    CProcessPipe* GetStderrPipe( void ) const;

    /** Closes the pipe to the standard input of the child: the child sees the end of its input */
    void CloseStdinPipe( void );

    /**
     *  With this on a process that is launched afterwards (Linux, macOS and other POSIX systems; Windows leaves this alone) is
     *  isolated from this one: its standard input, output and error are /dev/null, so it writes nothing to the terminal of
     *  this process and reads nothing from it, and no file descriptor of this process but those three is open in it (the
     *  sockets and the display connection of this process do not leak into it). The environment and the working directory
     *  are not touched. Together with 'detach' of Launch() the child has no terminal and no signals of ours either. Off by default.
     */
    void SetIsolateFromParent( bool isolate );

    bool GetIsolateFromParent( void ) const;

    /**
     *  Process id of the launched process, 0 if no process was launched
     */
    UInt32 GetProcessId( void ) const;

    /**
     *  Is the launched process still running? Also reaps the process if it has ended
     */
    bool IsRunning( void );

    /**
     *  Waits for the launched process to end
     *  @param timeoutInMs  maximum time to wait, GUCEF_UINT32MAX waits indefinitely
     *  @param exitCode     receives the exit code if the process ended. On Linux a process killed by a signal
     *                      reports 128 + the signal number
     *  @return true if the process has ended, false if it is still running when the timeout expired
     */
    bool WaitForExit( UInt32 timeoutInMs, Int32& exitCode );

    /**
     *  Asks the process to end and forces it to if it does not comply
     *  @param graceful         true: ask nicely first (SIGTERM / WM_CLOSE), then force after the grace period
     *                          false: force immediately (SIGKILL / TerminateProcess)
     *  @param gracePeriodInMs  time to wait for a graceful exit before forcing
     *  @return true if the process is no longer running
     */
    bool Terminate( bool graceful = true, UInt32 gracePeriodInMs = 5000 );

    /**
     *  Splits a command line into arguments the way a shell would for simple cases:
     *  whitespace separated, double or single quotes group words, a backslash escapes the next character inside double quotes
     */
    static TStringVector SplitCommandLine( const CString& commandLine );

    private:

    CProcess( const CProcess& src );
    CProcess& operator=( const CProcess& src );

    bool Reap( void );

    void ReleaseHandle( void );

    void ReleasePipes( void );

    bool LaunchImpl( const CString& exePath          ,
                     const TStringVector& args       ,
                     const CString& workingDir       ,
                     const TStringMap& envOverrides  ,
                     bool detach                     ,
                     UInt32 pipeMask                 );

    private:

    UInt32 m_processId;
    void* m_osHandle;       // Windows process handle, unused elsewhere
    bool m_isDetached;
    bool m_isolate;
    bool m_hasExited;
    Int32 m_exitCode;
    CProcessPipe* m_pipes[ 3 ];     // standard input, output and error
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPROCESS_H ? */
