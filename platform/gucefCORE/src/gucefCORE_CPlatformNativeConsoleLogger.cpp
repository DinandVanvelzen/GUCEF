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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include <stdlib.h>
#include <string.h>

#include "gucefCORE_CPlatformNativeConsoleLogger.h"

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

#ifndef GUCEF_CORE_CMSWINCONSOLELOGGER_H
#include "CMSWinConsoleLogger.h"
#define GUCEF_CORE_CMSWINCONSOLELOGGER_H
#endif /* GUCEF_CORE_CMSWINCONSOLELOGGER_H ? */

#elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX

#ifndef GUCEF_CORE_CXTERMCONSOLELOGGER_H
#include "CXTermConsoleLogger.h"
#define GUCEF_CORE_CXTERMCONSOLELOGGER_H
#endif /* GUCEF_CORE_CXTERMCONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CTERMINALCONSOLELOGGER_H
#include "gucefCORE_CTerminalConsoleLogger.h"
#define GUCEF_CORE_CTERMINALCONSOLELOGGER_H
#endif /* GUCEF_CORE_CTERMINALCONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H
#include "gucefCORE_CDesktopTerminalConsoleLogger.h"
#define GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H
#endif /* GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H ? */

#include <unistd.h>

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPlatformNativeConsoleLogger::CPlatformNativeConsoleLogger( bool ownWindow )
    : m_nativeConsoleLogger( GUCEF_NULL )
    , m_ownWindow( ownWindow )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlatformNativeConsoleLogger::EConsoleKind
CPlatformNativeConsoleLogger::ChooseConsoleKind( const char* forced, bool runningInTerminal, bool desktopSessionAvailable, bool ownWindow, bool stdoutClaimed )
{GUCEF_TRACE;

    if ( GUCEF_NULL != forced && '\0' != forced[ 0 ] && 0 != strcmp( forced, "auto" ) )
    {
        if ( 0 == strcmp( forced, "terminal" ) )
            return runningInTerminal ? CONSOLEKIND_TERMINAL : CONSOLEKIND_NONE;
        if ( 0 == strcmp( forced, "desktop" ) )
            return CONSOLEKIND_DESKTOP_TERMINAL;
        if ( 0 == strcmp( forced, "xterm" ) )
            return CONSOLEKIND_XTERM;
        if ( 0 == strcmp( forced, "stderr" ) )
            return CONSOLEKIND_STDERR;
        return CONSOLEKIND_NONE;        // "none" or anything we do not know
    }
    if ( stdoutClaimed )
    {
        // stdout carries a protocol: what a client that started the application with pipes shows is its stderr,
        // and a window of its own would be a surprise under such a client
        return CONSOLEKIND_STDERR;
    }
    if ( ownWindow )
    {
        // a window of its own; the terminal only when the desktop cannot give one
        if ( desktopSessionAvailable )
            return CONSOLEKIND_DESKTOP_TERMINAL;
        return runningInTerminal ? CONSOLEKIND_TERMINAL : CONSOLEKIND_NONE;
    }
    if ( runningInTerminal )
        return CONSOLEKIND_TERMINAL;
    if ( desktopSessionAvailable )
        return CONSOLEKIND_DESKTOP_TERMINAL;
    return CONSOLEKIND_NONE;            // headless, file and other loggers are not affected
}

/*-------------------------------------------------------------------------*/

CPlatformNativeConsoleLogger::~CPlatformNativeConsoleLogger()
{GUCEF_TRACE;

    // The logger may have been registered with the log manager: it must not be invoked after it is deleted
    if ( GUCEF_NULL != m_nativeConsoleLogger )
        CCoreGlobal::Instance()->GetLogManager().RemoveLogger( m_nativeConsoleLogger );
    GUCEF_DELETE m_nativeConsoleLogger;
    m_nativeConsoleLogger = NULL;
}

/*-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX

/**
 *  Linux has no console that every application gets, so what is best depends on how the application was started and on
 *  what it asks for: the terminal it was started from, or a terminal window of its own in the desktop session; in a
 *  headless environment there is no console to log to at all. See ChooseConsoleKind()
 */
static CIConsoleLogger*
CreateLinuxConsoleLogger( bool ownWindow )
{GUCEF_TRACE;

    const bool runningInTerminal = CTerminalConsoleLogger::IsRunningInTerminal();
    CPlatformNativeConsoleLogger::EConsoleKind kind =
        CPlatformNativeConsoleLogger::ChooseConsoleKind( getenv( "GUCEF_CONSOLE_LOGGER" ), runningInTerminal, CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable(), ownWindow, CStandardOutputWriter::IsClaimed() );

    if ( CPlatformNativeConsoleLogger::CONSOLEKIND_DESKTOP_TERMINAL == kind )
    {
        CDesktopTerminalConsoleLogger* logger = GUCEF_NEW CDesktopTerminalConsoleLogger();
        if ( logger->IsActive() )
            return logger;

        // no terminal emulator to be had or it is switched off: the terminal it was started from, if any
        GUCEF_DELETE logger;
        kind = runningInTerminal ? CPlatformNativeConsoleLogger::CONSOLEKIND_TERMINAL : CPlatformNativeConsoleLogger::CONSOLEKIND_NONE;
    }
    if ( CPlatformNativeConsoleLogger::CONSOLEKIND_TERMINAL == kind )
        return GUCEF_NEW CTerminalConsoleLogger();
    if ( CPlatformNativeConsoleLogger::CONSOLEKIND_XTERM == kind )
        return GUCEF_NEW CXTermConsoleLogger();
    if ( CPlatformNativeConsoleLogger::CONSOLEKIND_STDERR == kind )
        return GUCEF_NEW CTerminalConsoleLogger( STDERR_FILENO );
    return GUCEF_NULL;
}

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */

/*-------------------------------------------------------------------------*/

CIConsoleLogger*
CPlatformNativeConsoleLogger::GetLogger( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_nativeConsoleLogger )
    {
        #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
        // The console logger writes to the console of stdout and makes a console window when the process has none:
        // neither may happen while stdout carries a protocol
        if ( !CStandardOutputWriter::IsClaimed() )
            m_nativeConsoleLogger = GUCEF_NEW CMSWinConsoleLogger( m_ownWindow );
        #elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX
        m_nativeConsoleLogger = CreateLinuxConsoleLogger( m_ownWindow );
        #endif
    }
    return m_nativeConsoleLogger;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
