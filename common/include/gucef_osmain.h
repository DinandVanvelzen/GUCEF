/*
 *  gucef common header: provides header based platform wide facilities
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

 #ifndef GUCEF_OSMAIN_H
 #define GUCEF_OSMAIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <signal.h>
#include <string.h>

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#ifndef GUCEF_MEMORY_H
#include "gucef_memory.h"
#define GUCEF_MEMORY_H
#endif /* GUCEF_MEMORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_APP_TYPE_UNKNOWN                  0
#define GUCEF_APP_TYPE_BACKGROUND_PROCESSS      1
#define GUCEF_APP_TYPE_CONSOLE                  2
#define GUCEF_APP_TYPE_GUI                      3

/*-------------------------------------------------------------------------*/

/*
 *  You can override the App type in your project settings
 */
#undef GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS
#undef GUCEF_AUTO_APP_TYPE_CONSOLE
#undef GUCEF_AUTO_APP_TYPE_GUI
#ifdef GUCEF_AUTO_APP_TYPE_OVERRIDE
  #define GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS      GUCEF_AUTO_APP_TYPE_OVERRIDE
  #define GUCEF_AUTO_APP_TYPE_CONSOLE                  GUCEF_AUTO_APP_TYPE_OVERRIDE
  #define GUCEF_AUTO_APP_TYPE_GUI                      GUCEF_AUTO_APP_TYPE_OVERRIDE
#else
  #define GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS      GUCEF_APP_TYPE_BACKGROUND_PROCESSS
  #define GUCEF_AUTO_APP_TYPE_CONSOLE                  GUCEF_APP_TYPE_CONSOLE
  #define GUCEF_AUTO_APP_TYPE_GUI                      GUCEF_APP_TYPE_GUI
#endif

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_USE_MEMORY_LEAK_CHECKER
  
  #ifdef GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER

    #if ( !defined( GUCEF_DYNNEWON_DISABLED ) )    
      #define GUCEF_MEMORY_CHECK_APP_START { DRGUP_Initialize(); }
      #define GUCEF_MEMORY_CHECK_APP_END
    #endif

  #else
  
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) && ( GUCEF_COMPILER == GUCEF_COMPILER_MSVC ) )

      #define GUCEF_MEMORY_CHECK_APP_START
      #define GUCEF_MEMORY_CHECK_APP_END   { _CrtDumpMemoryLeaks(); }

    #endif

  #endif
#else    
  #define GUCEF_MEMORY_CHECK_APP_START
  #define GUCEF_MEMORY_CHECK_APP_END
#endif

/*-------------------------------------------------------------------------*/

/*
 *  Whether a command line given as one string holds the option --console as a whole token: delimited at both ends
 *  by the start or end of the string, a space, a tab, a '*' (the separator of the key / value pairs of the tools)
 *  or a double quote. "--console-log", "x--console" or a path that contains "--console" do not count.
 *  Used by the Windows entry point, which gets the arguments as one string; plain C so that it is tested on
 *  every platform.
 */
#ifdef __cplusplus
inline int
#else
static int
#endif
gucef_osmain_has_console_arg( const char* cmdline )
{
    static const char option[] = "--console";
    const size_t optionLength = sizeof( option ) - 1;
    const char* p = cmdline;
    while ( NULL != p && NULL != ( p = strstr( p, option ) ) )
    {
        const int startOk = ( p == cmdline ) || ' ' == p[ -1 ] || '\t' == p[ -1 ] || '*' == p[ -1 ] || '"' == p[ -1 ];
        const char after = p[ optionLength ];
        const int endOk = '\0' == after || ' ' == after || '\t' == after || '*' == after || '"' == after;
        if ( startOk && endOk )
            return 1;
        p += optionLength;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/*
 *  Here we will define the GUCEF_OSMAIN macro which defines a entry point
 *  and makes the following variables available and application parameters:
 *      int argc
 *      char** argv
 *
 *  This makes the parsing of application parameters uniform
 *
 *  Stop signals (SIGINT, SIGTERM; on Windows also the console close, break and shutdown events):
 *  - the handler given to GUCEF_OSMAIN_SIGNAL_HANDLER() runs in signal context (POSIX) or on a thread of the
 *    system (Windows console control). It must be async-signal-safe: the expected handler calls only
 *    GUCEF::CORE::CGUCEFApplication::RequestStopFromSignal( signal ), which the application acts on at its next
 *    pulse, on its own thread.
 *  - Linux and Android: GUCEF_OSSERVICEMAIN_BEGIN catches SIGINT and SIGTERM from the first statement of main()
 *    (a signal the parent made ignored stays ignored). A signal that comes before the application installed its
 *    handler is kept and handed to that handler the moment GUCEF_OSMAIN_SIGNAL_HANDLER() installs it, so a
 *    SIGTERM during the start of a service (for example while it loads its configuration, or to PID 1 of a
 *    container, for which the kernel has no default action) is not lost. GUCEF_OSMAIN_BEGIN keeps the default
 *    action (the process ends) for a signal before the handler is installed, unless the application says
 *    GUCEF_OSMAIN_DEFER_STOP_SIGNALS as its first statement.
 *  - Windows: the console control handler is installed by GUCEF_OSMAIN_SIGNAL_HANDLER() as before; a service
 *    stops on SERVICE_CONTROL_STOP and on SERVICE_CONTROL_SHUTDOWN (to verify on Windows): the control handler,
 *    on a thread of the service control manager, calls CGUCEFApplication::RequestStopFromSignal( SIGTERM ) and the
 *    application stops at its next pulse, on its own thread, as for a stop signal.
 */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #include <string.h>
    #include <windows.h>
    #undef min
    #undef max

    #define GUCEF_OSMAIN_BEGIN                                                                         \
                                                                                                       \
    static const unsigned char GUCEF_APP_TYPE = GUCEF_AUTO_APP_TYPE_CONSOLE;                           \
                                                                                                       \
    typedef void ( __cdecl *gucef_installed_signal_handler_ptr)( int );                                \
    static gucef_installed_signal_handler_ptr gucef_installed_signal_handler = NULL;                   \
                                                                                                       \
    void                                                                                               \
    gucef_signal_handler( int signal )                                                                 \
    {                                                                                                  \
        if ( NULL != gucef_installed_signal_handler )                                                  \
        {                                                                                              \
            (*gucef_installed_signal_handler)( signal );                                               \
        }                                                                                              \
    }                                                                                                  \
                                                                                                       \
    BOOL WINAPI Win32ConsoleHandlerRoutine( DWORD dwCtrlType )                                         \
    {                                                                                                  \
        if ( CTRL_C_EVENT == dwCtrlType  )                                                             \
        {                                                                                              \
            gucef_signal_handler( SIGINT );                                                            \
            return TRUE;                                                                               \
        }                                                                                              \
        else                                                                                           \
        if ( CTRL_BREAK_EVENT == dwCtrlType    ||                                                      \
             CTRL_CLOSE_EVENT == dwCtrlType    ||                                                      \
             CTRL_SHUTDOWN_EVENT == dwCtrlType )                                                       \
        {                                                                                              \
            gucef_signal_handler( SIGTERM );                                                           \
            return TRUE;                                                                               \
        }                                                                                              \
        return FALSE;                                                                                  \
    }                                                                                                  \
                                                                                                       \
    int WINAPI                                                                                         \
    WinMain( HINSTANCE hinstance     ,                                                                 \
             HINSTANCE hprevinstance ,                                                                 \
             LPSTR lpcmdline         ,                                                                 \
             int ncmdshow            )                                                                 \
    {                                                                                                  \
        GUCEF_MEMORY_CHECK_APP_START;                                                                  \
                                                                                                       \
        int argc = 0;                                                                                  \
        char** argv = &lpcmdline;                                                                      \
        if ( lpcmdline != NULL )                                                                       \
        {                                                                                              \
            if ( *lpcmdline != '\0' )                                                                  \
            {                                                                                          \
                argc = 1;                                                                              \
            }                                                                                          \
        }

    #define GUCEF_OSMAIN_END    GUCEF_MEMORY_CHECK_APP_END}

    #define GUCEF_OSSERVICEMAIN_BEGIN( serviceName )                                                   \
                                                                                                       \
    static unsigned char GUCEF_APP_TYPE = GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS;                     \
                                                                                                       \
    typedef void ( __cdecl *gucef_installed_signal_handler_ptr)( int );                                \
    static gucef_installed_signal_handler_ptr gucef_installed_signal_handler = NULL;                   \
                                                                                                       \
    void                                                                                               \
    gucef_signal_handler( int signal )                                                                 \
    {                                                                                                  \
        if ( NULL != gucef_installed_signal_handler )                                                  \
        {                                                                                              \
            (*gucef_installed_signal_handler)( signal );                                               \
        }                                                                                              \
    }                                                                                                  \
                                                                                                       \
    BOOL WINAPI Win32ConsoleHandlerRoutine( DWORD dwCtrlType )                                         \
    {                                                                                                  \
        if ( CTRL_C_EVENT == dwCtrlType  )                                                             \
        {                                                                                              \
            gucef_signal_handler( SIGINT );                                                            \
            return TRUE;                                                                               \
        }                                                                                              \
        else                                                                                           \
        if ( CTRL_BREAK_EVENT == dwCtrlType    ||                                                      \
             CTRL_CLOSE_EVENT == dwCtrlType    ||                                                      \
             CTRL_SHUTDOWN_EVENT == dwCtrlType )                                                       \
        {                                                                                              \
            gucef_signal_handler( SIGTERM );                                                           \
            return TRUE;                                                                               \
        }                                                                                              \
        return FALSE;                                                                                  \
    }                                                                                                  \
                                                                                                       \
    ::SERVICE_STATUS_HANDLE g_win32ServiceStatusHandle = NULL;                                         \
    ::HANDLE g_win32StopEvent = NULL;                                                                  \
    ::DWORD g_win32CurrentState = 0;                                                                   \
    bool g_win32SystemShutdown = false;                                                                \
                                                                                                       \
    void WINAPI Win32ServiceMain( DWORD argc, LPSTR* argv );                                           \
    int Win32ServiceInitializer( int argc, char** argv );                                              \
                                                                                                       \
    int __stdcall                                                                                      \
    WinMain( HINSTANCE hinstance     ,                                                                 \
             HINSTANCE hprevinstance ,                                                                 \
             LPSTR lpcmdline         ,                                                                 \
             int ncmdshow            )                                                                 \
    {                                                                                                  \
                                                                                                       \
        int argc = 0;                                                                                  \
        char** argv = &lpcmdline;                                                                      \
        if ( lpcmdline != NULL )                                                                       \
        {                                                                                              \
            if ( *lpcmdline != '\0' )                                                                  \
            {                                                                                          \
                argc = 1;                                                                              \
                if ( 0 != gucef_osmain_has_console_arg( lpcmdline ) )                                  \
                {                                                                                      \
                    GUCEF_APP_TYPE = GUCEF_APP_TYPE_CONSOLE;                                           \
                    return Win32ServiceInitializer( argc, argv );                                      \
                }                                                                                      \
            }                                                                                          \
        }                                                                                              \
                                                                                                       \
        char win32ServiceTableName[] = "";                                                             \
        ::SERVICE_TABLE_ENTRYA win32ServiceTable[] = {                                                 \
            { win32ServiceTableName, &Win32ServiceMain },                                              \
            { NULL, NULL }                                                                             \
        };                                                                                             \
                                                                                                       \
        if ( ::StartServiceCtrlDispatcherA( win32ServiceTable ) )                                      \
            return 0;                                                                                  \
        else if ( ERROR_FAILED_SERVICE_CONTROLLER_CONNECT == ::GetLastError() )                        \
            return -1;                                                                                 \
        else                                                                                           \
            return -2;                                                                                 \
    }                                                                                                  \
                                                                                                       \
    void                                                                                               \
    Win32ReportErrorStatus( DWORD errorCode )                                                          \
    {                                                                                                  \
        g_win32CurrentState = SERVICE_STOPPED;                                                         \
        SERVICE_STATUS serviceStatus = {                                                               \
            SERVICE_WIN32_OWN_PROCESS,                                                                 \
            g_win32CurrentState,                                                                       \
            0,                                                                                         \
            ERROR_SERVICE_SPECIFIC_ERROR,                                                              \
            errorCode,                                                                                 \
            0,                                                                                         \
            0,                                                                                         \
        };                                                                                             \
        ::SetServiceStatus( g_win32ServiceStatusHandle, &serviceStatus );                              \
    }                                                                                                  \
                                                                                                       \
    void                                                                                               \
    Win32ReportStatus( DWORD state )                                                                   \
    {                                                                                                  \
        g_win32CurrentState = state;                                                                   \
        SERVICE_STATUS serviceStatus = {                                                               \
            SERVICE_WIN32_OWN_PROCESS,                                                                 \
            g_win32CurrentState,                                                                       \
            state == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,        \
            NO_ERROR,                                                                                  \
            0,                                                                                         \
            0,                                                                                         \
            0,                                                                                         \
        };                                                                                             \
        ::SetServiceStatus( g_win32ServiceStatusHandle, &serviceStatus );                              \
    }                                                                                                  \
                                                                                                       \
    DWORD WINAPI Win32HandlerEx( DWORD control, DWORD eventType, void* eventData, void* context )      \
    {                                                                                                  \
        switch ( control )                                                                             \
        {                                                                                              \
            case SERVICE_CONTROL_SHUTDOWN:                                                             \
            case SERVICE_CONTROL_STOP:                                                                 \
            {                                                                                          \
                /* at system shutdown the application stops as for a stop, within the SCM's time */    \
                if ( SERVICE_CONTROL_SHUTDOWN == control )                                             \
                    g_win32SystemShutdown = true;                                                      \
                Win32ReportStatus( SERVICE_STOP_PENDING );                                             \
                ::SetEvent( g_win32StopEvent );                                                        \
                /* the application stops on its own pulse thread, not on this SCM thread */            \
                ::GUCEF::CORE::CGUCEFApplication::RequestStopFromSignal( SIGTERM );                    \
                break;                                                                                 \
            }                                                                                          \
            default:                                                                                   \
            {                                                                                          \
                Win32ReportStatus( g_win32CurrentState );                                              \
                break;                                                                                 \
            }                                                                                          \
        }                                                                                              \
        return NO_ERROR;                                                                               \
    }                                                                                                  \
                                                                                                       \
    void WINAPI Win32ServiceMain( DWORD argc, LPSTR *argv )                                            \
    {                                                                                                  \
        GUCEF_MEMORY_CHECK_APP_START;                                                                  \
                                                                                                       \
        g_win32ServiceStatusHandle =                                                                   \
            ::RegisterServiceCtrlHandlerExA( serviceName, &Win32HandlerEx, NULL);                      \
                                                                                                       \
        Win32ReportStatus( SERVICE_RUNNING );                                                          \
        g_win32StopEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );                                   \
        int appResult = Win32ServiceInitializer( (int) argc, argv );                                   \
        if ( 0 != g_win32StopEvent )                                                                   \
            ::CloseHandle( g_win32StopEvent );                                                         \
                                                                                                       \
        if ( appResult != 0 )                                                                          \
            Win32ReportErrorStatus( (DWORD)appResult );                                                \
        else                                                                                           \
            Win32ReportStatus( SERVICE_STOPPED );                                                      \
    }                                                                                                  \
                                                                                                       \
    int                                                                                                \
    Win32ServiceInitializer( int argc, char** argv )                                                   \
    {

    #define GUCEF_OSSERVICEMAIN_END     GUCEF_MEMORY_CHECK_APP_END}

    #define GUCEF_OSMAIN_SIGNAL_HANDLER( signalHandlerFunc )                                           \
    {                                                                                                  \
        if ( GUCEF_APP_TYPE_CONSOLE == GUCEF_APP_TYPE )                                                \
            ::SetConsoleCtrlHandler( &Win32ConsoleHandlerRoutine, TRUE );                              \
        gucef_installed_signal_handler = &signalHandlerFunc;                                           \
        signal( SIGINT, &signalHandlerFunc );                                                          \
        signal( SIGTERM, &signalHandlerFunc );                                                         \
    }

    /* Linux and Android only: the Windows entry points keep the default handling until the handler is installed */
    #define GUCEF_OSMAIN_DEFER_STOP_SIGNALS

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    #include <string.h>
    #include <unistd.h>

    #define GUCEF_OSMAIN_BEGIN_APPTYPE_SIGNALS( appType, deferStopSignals )                            \
                                                                                                       \
    static unsigned char GUCEF_APP_TYPE = appType;                                                     \
    typedef void ( *gucef_installed_signal_handler_ptr)( int );                                        \
    static gucef_installed_signal_handler_ptr gucef_installed_signal_handler = GUCEF_NULL;             \
    static volatile sig_atomic_t gucef_osmain_pending_signal = 0;                                      \
    static volatile sig_atomic_t gucef_osmain_defer_stop_signals = deferStopSignals;                   \
                                                                                                       \
    void linux_sig_term_handler( int signum, siginfo_t* info, void* ptr )                              \
    {                                                                                                  \
        if ( NULL != gucef_installed_signal_handler )                                                  \
            gucef_installed_signal_handler( signum );                                                  \
    }                                                                                                  \
                                                                                                       \
    /* until the application installs its handler; async-signal-safe */                               \
    static void gucef_osmain_early_signal_handler( int signum )                                        \
    {                                                                                                  \
        if ( 0 != gucef_osmain_defer_stop_signals )                                                    \
        {                                                                                              \
            gucef_osmain_pending_signal = signum;                                                      \
            return;                                                                                    \
        }                                                                                              \
        /* the default action, as if no handler had been installed */                                  \
        signal( signum, SIG_DFL );                                                                     \
        raise( signum );                                                                               \
    }                                                                                                  \
                                                                                                       \
    static void gucef_osmain_install_early_signal_handler( int signum )                                \
    {                                                                                                  \
        struct sigaction current;                                                                      \
        memset( &current, 0, sizeof( current ) );                                                      \
        if ( 0 != sigaction( signum, NULL, &current ) || SIG_IGN == current.sa_handler )               \
            return;                                                                                    \
        struct sigaction early;                                                                        \
        memset( &early, 0, sizeof( early ) );                                                          \
        early.sa_handler = gucef_osmain_early_signal_handler;                                          \
        sigemptyset( &early.sa_mask );                                                                 \
        sigaction( signum, &early, NULL );                                                             \
    }                                                                                                  \
                                                                                                       \
    int                                                                                                \
    main( int argc, char* argv[] )                                                                     \
    {                                                                                                  \
        gucef_osmain_install_early_signal_handler( SIGTERM );                                          \
        gucef_osmain_install_early_signal_handler( SIGINT );                                           \
        if ( 0 != argc && argv != GUCEF_NULL )                                                         \
        {                                                                                              \
            for ( int i=0; i<argc; ++i )                                                               \
            {                                                                                          \
                if ( 0 == strcmp( argv[ i ], "--console" ) )                                           \
                {                                                                                      \
                    GUCEF_APP_TYPE = GUCEF_APP_TYPE_CONSOLE;                                           \
                    break;                                                                             \
                }                                                                                      \
            }                                                                                          \
        }

    #define GUCEF_OSMAIN_BEGIN_APPTYPE( appType )       GUCEF_OSMAIN_BEGIN_APPTYPE_SIGNALS( appType, 0 )

    #define GUCEF_OSMAIN_BEGIN      GUCEF_OSMAIN_BEGIN_APPTYPE( GUCEF_AUTO_APP_TYPE_CONSOLE )

    #define GUCEF_OSMAIN_END }

    #define GUCEF_OSSERVICEMAIN_BEGIN( serviceName )    GUCEF_OSMAIN_BEGIN_APPTYPE_SIGNALS( GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS, 1 )
    #define GUCEF_OSSERVICEMAIN_END                     GUCEF_OSMAIN_END

    /* a stop signal before GUCEF_OSMAIN_SIGNAL_HANDLER() is kept for the handler, as GUCEF_OSSERVICEMAIN_BEGIN does */
    #define GUCEF_OSMAIN_DEFER_STOP_SIGNALS     { gucef_osmain_defer_stop_signals = 1; }

    #define GUCEF_OSMAIN_SIGNAL_HANDLER( signalHandlerFunc )                                           \
    {                                                                                                  \
        gucef_installed_signal_handler = &signalHandlerFunc;                                           \
                                                                                                       \
        static struct sigaction _sigact;                                                               \
        memset( &_sigact, 0, sizeof(_sigact) );                                                        \
        _sigact.sa_flags = SA_SIGINFO;                                                                 \
        _sigact.sa_sigaction = linux_sig_term_handler;                                                 \
                                                                                                       \
        sigaction( SIGINT, &_sigact, NULL );                                                           \
        sigaction( SIGTERM, &_sigact, NULL );                                                          \
                                                                                                       \
        /* a signal that came before the handler: handed to it now, outside signal context */          \
        if ( 0 != gucef_osmain_pending_signal )                                                        \
        {                                                                                              \
            int _pendingSignal = (int) gucef_osmain_pending_signal;                                    \
            gucef_osmain_pending_signal = 0;                                                           \
            signalHandlerFunc( _pendingSignal );                                                       \
        }                                                                                              \
    }

#else

    #define GUCEF_OSMAIN_BEGIN_APPTYPE( appType )                                                      \
                                                                                                       \
    static const unsigned char GUCEF_APP_TYPE = appType;                                               \
    typedef void ( *gucef_installed_signal_handler_ptr)( int );                                        \
    static gucef_installed_signal_handler_ptr gucef_installed_signal_handler = NULL;                   \
                                                                                                       \
    int                                                                                                \
    main( int argc, char* argv[] )                                                                     \
    {

    #define GUCEF_OSMAIN_BEGIN      GUCEF_OSMAIN_BEGIN_APPTYPE( GUCEF_AUTO_APP_TYPE_CONSOLE )

    #define GUCEF_OSMAIN_END }

    #define GUCEF_OSSERVICEMAIN_BEGIN( serviceName )    GUCEF_OSMAIN_BEGIN_APPTYPE( GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS )
    #define GUCEF_OSSERVICEMAIN_END                     GUCEF_OSMAIN_END

    #define GUCEF_OSMAIN_SIGNAL_HANDLER( signalHandlerFunc )

    #define GUCEF_OSMAIN_DEFER_STOP_SIGNALS

#endif

/*-------------------------------------------------------------------------*/

 #endif /* GUCEF_OSMAIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-01-2007 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
