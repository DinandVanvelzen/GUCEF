/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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
 *  Here we will define the GUCEF_OSMAIN macro which defines a entry point
 *  and makes the following variables available and application parameters:
 *      int argc
 *      char** argv
 *
 *  This makes the parsing of application parameters uniform
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
    void WINAPI Win32ServiceMain( DWORD argc, LPTSTR* argv );                                          \
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
                if ( NULL != strstr( lpcmdline, "--console" ) )                                        \
                {                                                                                      \
                    GUCEF_APP_TYPE = GUCEF_APP_TYPE_CONSOLE;                                           \
                    return Win32ServiceInitializer( argc, argv );                                      \
                }                                                                                      \
            }                                                                                          \
        }                                                                                              \
                                                                                                       \
        ::SERVICE_TABLE_ENTRY win32ServiceTable[] = {                                                  \
            { "", &Win32ServiceMain },                                                                 \
            { NULL, NULL }                                                                             \
        };                                                                                             \
                                                                                                       \
        if ( ::StartServiceCtrlDispatcher( win32ServiceTable ) )                                       \
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
            {                                                                                          \
                g_win32SystemShutdown = true;                                                          \
                break;                                                                                 \
            }                                                                                          \
            case SERVICE_CONTROL_STOP:                                                                 \
            {                                                                                          \
                Win32ReportStatus( SERVICE_STOP_PENDING );                                             \
                ::SetEvent( g_win32StopEvent );                                                        \
                ::GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();                       \
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
    void WINAPI Win32ServiceMain( DWORD argc, LPTSTR *argv )                                           \
    {                                                                                                  \
        GUCEF_MEMORY_CHECK_APP_START;                                                                  \
                                                                                                       \
        g_win32ServiceStatusHandle =                                                                   \
            ::RegisterServiceCtrlHandlerEx( serviceName, &Win32HandlerEx, NULL);                       \
                                                                                                       \
        Win32ReportStatus( SERVICE_RUNNING );                                                          \
        g_win32StopEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );                                   \
        int appResult = Win32ServiceInitializer( argc, argv );                                         \
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

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    #include <string.h>
    #include <unistd.h>

    #define GUCEF_OSMAIN_BEGIN_APPTYPE( appType )                                                      \
                                                                                                       \
    static unsigned char GUCEF_APP_TYPE = appType;                                                     \
    typedef void ( *gucef_installed_signal_handler_ptr)( int );                                        \
    static gucef_installed_signal_handler_ptr gucef_installed_signal_handler = GUCEF_NULL;             \
                                                                                                       \
    void linux_sig_term_handler( int signum, siginfo_t* info, void* ptr )                              \
    {                                                                                                  \
        if ( NULL != gucef_installed_signal_handler )                                                  \
            gucef_installed_signal_handler( signum );                                                  \
    }                                                                                                  \
                                                                                                       \
    int                                                                                                \
    main( int argc, char* argv[] )                                                                     \
    {                                                                                                  \
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

    #define GUCEF_OSMAIN_BEGIN      GUCEF_OSMAIN_BEGIN_APPTYPE( GUCEF_AUTO_APP_TYPE_CONSOLE )

    #define GUCEF_OSMAIN_END }

    #define GUCEF_OSSERVICEMAIN_BEGIN( serviceName )    GUCEF_OSMAIN_BEGIN_APPTYPE( GUCEF_AUTO_APP_TYPE_BACKGROUND_PROCESSS )
    #define GUCEF_OSSERVICEMAIN_END                     GUCEF_OSMAIN_END

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
