/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

    private:

    UInt32 m_processId;
    void* m_osHandle;       // Windows process handle, unused elsewhere
    bool m_isDetached;
    bool m_isolate;
    bool m_hasExited;
    Int32 m_exitCode;
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
