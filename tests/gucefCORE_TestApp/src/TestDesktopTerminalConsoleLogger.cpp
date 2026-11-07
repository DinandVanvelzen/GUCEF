/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifdef GUCEF_LINUX_BUILD

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H
#include "gucefCORE_CDesktopTerminalConsoleLogger.h"
#define GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H
#endif /* GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CTERMINALCONSOLELOGGER_H
#include "gucefCORE_CTerminalConsoleLogger.h"
#define GUCEF_CORE_CTERMINALCONSOLELOGGER_H
#endif /* GUCEF_CORE_CTERMINALCONSOLELOGGER_H ? */

#endif /* GUCEF_LINUX_BUILD ? */

#include "TestDesktopTerminalConsoleLogger.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#ifdef GUCEF_LINUX_BUILD

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Changes environment variables for the lifetime of the object, restoring them in the end */
class CEnvironmentScope
{
    public:

    CEnvironmentScope( void ) {}

    ~CEnvironmentScope()
    {
        for ( size_t i=0; i<m_saved.size(); ++i )
        {
            const SSaved& saved = m_saved[ i ];    // const: only the const C_String() never returns NULL, the value can be empty
            if ( saved.wasSet )
                setenv( saved.name.C_String(), saved.value.C_String(), 1 );
            else
                unsetenv( saved.name.C_String() );
        }
    }

    void Set( const char* name, const char* value )
    {
        Remember( name );
        setenv( name, value, 1 );
    }

    void Unset( const char* name )
    {
        Remember( name );
        unsetenv( name );
    }

    private:

    struct SSaved { CORE::CString name; CORE::CString value; bool wasSet; };

    void Remember( const char* name )
    {
        for ( size_t i=0; i<m_saved.size(); ++i )
            if ( m_saved[ i ].name == name )
                return;
        SSaved saved;
        saved.name = name;
        const char* current = getenv( name );
        saved.wasSet = GUCEF_NULL != current;
        if ( saved.wasSet )
            saved.value = current;
        m_saved.push_back( saved );
    }

    GUCEF::vector< SSaved > m_saved;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
WriteExecutable( const CORE::CString& path, const CORE::CString& content )
{
    if ( !CORE::WriteStringAsTextFile( path, content ) )
        return false;
    return 0 == chmod( path.C_String(), 0755 );
}

/*-------------------------------------------------------------------------*/

static bool
WaitForFileContent( const CORE::CString& path, const CORE::CString& expected, CORE::CDesktopTerminalConsoleLogger* pump, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=10 )
    {
        CORE::CString content;
        if ( CORE::LoadTextFileAsString( path, content, false ) && content.HasSubstr( expected, true ) >= 0 )
            return true;
        if ( GUCEF_NULL != pump )
            pump->FlushLog();
        MT::ThreadDelay( 10 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static void
LogLine( CORE::CILogger& logger, const CORE::CString& text )
{
    logger.LogWithoutFormatting( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, text, 0, CORE::CTimestamp::NowUTCTime() );
}

#endif /* GUCEF_LINUX_BUILD ? */

/*-------------------------------------------------------------------------*/

void
PerformDesktopTerminalConsoleLoggerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING desktop terminal console logger TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DesktopTerminalConsoleLogger" );

    #ifdef GUCEF_LINUX_BUILD

    TESTFW::CScopedTestDir scopedWorkDir( "gucefCORE_TestApp_ConsoleLogger" );
    CORE::CString workDir = scopedWorkDir.GetPath();
    CORE::CString binDir = CORE::CombinePath( workDir, "bin" );
    CORE::CString emptyDir = CORE::CombinePath( workDir, "empty" );
    CORE::CString outPath = CORE::CombinePath( workDir, "terminal.out" );
    CORE::CreateDirs( binDir );
    CORE::CreateDirs( emptyDir );

    // The path of the fake terminal is searched using PATH, the shell and cat of the viewer need the real one
    const char* realPath = getenv( "PATH" );
    CORE::CString searchPathWithFakes = binDir + ":" + CORE::CString( GUCEF_NULL != realPath ? realPath : "/usr/bin:/bin" );

    // Fake terminals. Like a real one it runs the command it is given, and what the command shows goes to a file.
    // This one understands "-e command".
    CORE::CString fakeXterm =
        "#!/bin/sh\n"
        "while [ \"$1\" != \"-e\" ] && [ $# -gt 0 ]; do shift; done\n"
        "shift\n"
        "exec \"$@\" > \"$GUCEF_FAKE_TERMINAL_OUT\"\n";
    // This one understands "--title title command" (kitty style) and records how it was called
    CORE::CString fakeKitty =
        "#!/bin/sh\n"
        "echo \"$@\" > \"$GUCEF_FAKE_TERMINAL_ARGS\"\n"
        "shift 2\n"
        "exec \"$@\" > \"$GUCEF_FAKE_TERMINAL_OUT\"\n";

    GUCEF_TESTFW_TESTCASE( "Test: A terminal emulator is discovered, in order of preference, and told what to run" )
        try
        {
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ), fakeXterm ) );
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "kitty" ), fakeKitty ) );
            CORE::CProcess::TStringVector command;
            command.push_back( "viewer" );
            command.push_back( "arg1" );

            // only the fake terminals can be found, the ones of the machine we run on must not play a role
            CEnvironmentScope environment;
            environment.Set( "PATH", binDir.C_String() );
            environment.Unset( "TERMINAL" );

            // the list of well known terminals puts kitty before xterm
            CORE::CString exePath, terminalName;
            CORE::CProcess::TStringVector args;
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::DiscoverTerminal( "", "My Title", command, exePath, args, terminalName ) );
            ASSERT_TRUE( "kitty" == terminalName );
            ASSERT_TRUE( CORE::CombinePath( binDir, "kitty" ) == exePath );
            ASSERT_TRUE( 4 == args.size() && "--title" == args[ 0 ] && "My Title" == args[ 1 ] && "viewer" == args[ 2 ] && "arg1" == args[ 3 ] );

            // a preferred terminal wins and gets its own syntax
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::DiscoverTerminal( "xterm", "My Title", command, exePath, args, terminalName ) );
            ASSERT_TRUE( "xterm" == terminalName );
            ASSERT_TRUE( 5 == args.size() && "-T" == args[ 0 ] && "My Title" == args[ 1 ] && "-e" == args[ 2 ] && "viewer" == args[ 3 ] );

            // as does the terminal of the user, an unknown one is expected to take -e
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "mytermy" ), fakeXterm ) );
            environment.Set( "TERMINAL", "mytermy" );
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::DiscoverTerminal( "", "My Title", command, exePath, args, terminalName ) );
            ASSERT_TRUE( "mytermy" == terminalName );
            ASSERT_TRUE( 3 == args.size() && "-e" == args[ 0 ] && "viewer" == args[ 1 ] );

            // a preferred terminal that is not there is skipped
            environment.Unset( "TERMINAL" );
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::DiscoverTerminal( "nosuchterminalatall", "T", command, exePath, args, terminalName ) );
            ASSERT_TRUE( "kitty" == terminalName );

            // and when there is nothing, there is nothing
            environment.Set( "PATH", emptyDir.C_String() );
            ASSERT_FALSE( CORE::CDesktopTerminalConsoleLogger::DiscoverTerminal( "", "T", command, exePath, args, terminalName ) );
            ASSERT_FALSE( CORE::CDesktopTerminalConsoleLogger::FindExecutableInPath( "xterm", exePath ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The log is shown in the terminal window, the title is set and the window ends with the log" )
        try
        {
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ), fakeXterm ) );
            CORE::DeleteFile( outPath );

            CEnvironmentScope environment;
            environment.Set( "PATH", searchPathWithFakes.C_String() );
            environment.Set( "GUCEF_FAKE_TERMINAL_OUT", outPath.C_String() );
            environment.Unset( "TERMINAL" );

            {
                CORE::CDesktopTerminalConsoleLogger logger( false );
                logger.SetPreferredTerminal( "xterm" );
                logger.SetWindowTitle( "Test Log Window" );
                ASSERT_TRUE( logger.Initialize() );
                ASSERT_TRUE( logger.IsActive() );
                ASSERT_TRUE( "xterm" == logger.GetTerminalName() );

                // lines logged while the window is still starting are kept for it
                LogLine( logger, "first line" );
                LogLine( logger, "second line" );
                ASSERT_TRUE( WaitForFileContent( outPath, "second line", &logger, 10000 ) );
                ASSERT_TRUE( logger.IsConnected() );

                LogLine( logger, "third line" );
                ASSERT_TRUE( WaitForFileContent( outPath, "third line", &logger, 10000 ) );

                CORE::CString shown;
                ASSERT_TRUE( CORE::LoadTextFileAsString( outPath, shown, false ) );
                ASSERT_TRUE( shown.HasSubstr( "Test Log Window", true ) >= 0 );
                Int32 first = shown.HasSubstr( "first line", true );
                Int32 second = shown.HasSubstr( "second line", true );
                Int32 third = shown.HasSubstr( "third line", true );
                ASSERT_TRUE( first >= 0 && second > first && third > second );
                ASSERT_TRUE( 0 == logger.GetDroppedLineCount() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without a terminal emulator there is no window and logging does not hold anything up" )
        try
        {
            CEnvironmentScope environment;
            environment.Set( "PATH", emptyDir.C_String() );
            environment.Unset( "TERMINAL" );

            CORE::CDesktopTerminalConsoleLogger logger( false );
            ASSERT_FALSE( logger.Initialize() );
            ASSERT_FALSE( logger.IsActive() );

            UInt64 start = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<1000; ++i )
                LogLine( logger, "nobody sees this" );
            Float64 elapsedInMs = ( MT::PrecisionTickCount() - start ) / ( MT::PrecisionTimerResolution() / 1000.0 );
            ASSERT_TRUE( elapsedInMs < 1000.0 );
            ASSERT_FALSE( logger.IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A terminal window that never shows up is given up on" )
        try
        {
            // this terminal exits without ever opening the log
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ), "#!/bin/sh\nexit 0\n" ) );

            CEnvironmentScope environment;
            environment.Set( "PATH", searchPathWithFakes.C_String() );
            environment.Unset( "TERMINAL" );

            CORE::CDesktopTerminalConsoleLogger logger( false );
            logger.SetPreferredTerminal( "xterm" );
            logger.SetConnectTimeoutInMs( 300 );
            ASSERT_TRUE( logger.Initialize() );

            LogLine( logger, "waiting for a window" );
            ASSERT_TRUE( logger.IsActive() );
            ASSERT_FALSE( logger.IsConnected() );

            MT::ThreadDelay( 400 );
            LogLine( logger, "still waiting?" );
            ASSERT_FALSE( logger.IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A terminal window that is closed by the user disables the logger without hurting the application" )
        try
        {
            // this terminal closes after having shown 40 bytes, which is what closing the window looks like to the logger
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ),
                "#!/bin/sh\n"
                "while [ \"$1\" != \"-e\" ] && [ $# -gt 0 ]; do shift; done\n"
                "shift\n"
                "\"$@\" | head -c 40 > \"$GUCEF_FAKE_TERMINAL_OUT\"\n" ) );
            CORE::DeleteFile( outPath );

            CEnvironmentScope environment;
            environment.Set( "PATH", searchPathWithFakes.C_String() );
            environment.Set( "GUCEF_FAKE_TERMINAL_OUT", outPath.C_String() );
            environment.Unset( "TERMINAL" );

            CORE::CDesktopTerminalConsoleLogger logger( false );
            logger.SetPreferredTerminal( "xterm" );
            ASSERT_TRUE( logger.Initialize() );

            // keep logging until the window is gone, which must not kill us with a SIGPIPE
            bool disabled = false;
            for ( UInt32 i=0; i<500 && !disabled; ++i )
            {
                LogLine( logger, "a line of text to fill up the window quickly, " + CORE::UInt32ToString( i ) );
                MT::ThreadDelay( 10 );
                disabled = !logger.IsActive();
            }
            ASSERT_TRUE( disabled );

            // and it stays quiet from then on
            LogLine( logger, "after the window is gone" );
            ASSERT_FALSE( logger.IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: When the window cannot keep up the oldest lines are dropped, and said to be" )
        try
        {
            // this terminal is up but does not read the log
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ), "#!/bin/sh\nexec sleep 20\n" ) );

            CEnvironmentScope environment;
            environment.Set( "PATH", searchPathWithFakes.C_String() );
            environment.Unset( "TERMINAL" );

            CORE::CDesktopTerminalConsoleLogger logger( false );
            logger.SetPreferredTerminal( "xterm" );
            logger.SetMaxBufferedBytes( 1000 );
            logger.SetConnectTimeoutInMs( 60000 );
            ASSERT_TRUE( logger.Initialize() );

            for ( UInt32 i=0; i<200; ++i )
                LogLine( logger, "line number " + CORE::UInt32ToString( i ) + " which is rather long to fill the buffer" );

            ASSERT_TRUE( logger.GetDroppedLineCount() > 100 );
            ASSERT_TRUE( logger.IsActive() );
            // the destructor ends the terminal that never showed the log
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The terminal logger writes to the terminal and drops lines instead of blocking when it is stalled" )
        try
        {
            int pipeFds[ 2 ];
            ASSERT_TRUE( 0 == pipe( pipeFds ) );
            fcntl( pipeFds[ 1 ], F_SETFL, fcntl( pipeFds[ 1 ], F_GETFL, 0 ) | O_NONBLOCK );

            {
                CORE::CTerminalConsoleLogger logger( pipeFds[ 1 ] );
                LogLine( logger, "hello terminal" );
                char buffer[ 100 ];
                ssize_t bytesRead = read( pipeFds[ 0 ], buffer, sizeof( buffer ) );
                ASSERT_TRUE( 15 == bytesRead && 0 == memcmp( buffer, "hello terminal\n", 15 ) );
                ASSERT_TRUE( 0 == logger.GetDroppedLineCount() );

                // nobody reads anymore: the pipe fills up and then lines are dropped
                CORE::CString filler = "a long line of text which takes up space in the pipe, ";
                for ( UInt32 x=0; x<200; ++x )
                    filler += 'x';
                UInt64 start = MT::PrecisionTickCount();
                for ( UInt32 i=0; i<2000; ++i )
                    LogLine( logger, filler );
                Float64 elapsedInMs = ( MT::PrecisionTickCount() - start ) / ( MT::PrecisionTimerResolution() / 1000.0 );
                ASSERT_TRUE( elapsedInMs < 3000.0 );
                ASSERT_TRUE( logger.GetDroppedLineCount() > 0 );
            }
            close( pipeFds[ 0 ] );
            close( pipeFds[ 1 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No terminal window pops up in a headless environment, and the logger for the environment can be forced" )
        try
        {
            CEnvironmentScope environment;
            environment.Unset( "WAYLAND_DISPLAY" );
            environment.Unset( "DISPLAY" );
            environment.Unset( "XDG_SESSION_TYPE" );
            ASSERT_FALSE( CORE::CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable() );

            environment.Set( "DISPLAY", ":0" );
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable() );
            environment.Unset( "DISPLAY" );
            environment.Set( "WAYLAND_DISPLAY", "wayland-0" );
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable() );
            environment.Unset( "WAYLAND_DISPLAY" );
            environment.Set( "XDG_SESSION_TYPE", "tty" );
            ASSERT_FALSE( CORE::CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable() );
            environment.Set( "XDG_SESSION_TYPE", "wayland" );
            ASSERT_TRUE( CORE::CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable() );
            environment.Unset( "XDG_SESSION_TYPE" );

            // forced to none there is no console logger, what GetLogger() reports
            environment.Set( "GUCEF_CONSOLE_LOGGER", "none" );
            {
                CORE::CPlatformNativeConsoleLogger console;
                ASSERT_TRUE( GUCEF_NULL == console.GetLogger() );
            }

            // forced to desktop while there is no terminal emulator to be had there is no console logger either
            environment.Set( "GUCEF_CONSOLE_LOGGER", "desktop" );
            environment.Set( "PATH", emptyDir.C_String() );
            environment.Unset( "TERMINAL" );
            {
                CORE::CPlatformNativeConsoleLogger console;
                ASSERT_TRUE( GUCEF_NULL == console.GetLogger() );
            }

            // not in a terminal and not on a desktop: nothing, in auto mode
            environment.Set( "GUCEF_CONSOLE_LOGGER", "auto" );
            if ( !CORE::CTerminalConsoleLogger::IsRunningInTerminal() )
            {
                CORE::CPlatformNativeConsoleLogger console;
                ASSERT_TRUE( GUCEF_NULL == console.GetLogger() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The log manager switches its console log window on and off while the application runs" )
        try
        {
            // the window is a fake terminal that writes what it shows to a file
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ), fakeXterm ) );
            CORE::DeleteFile( outPath );
            CEnvironmentScope environment;
            environment.Set( "PATH", searchPathWithFakes.C_String() );
            environment.Set( "GUCEF_FAKE_TERMINAL_OUT", outPath.C_String() );
            environment.Set( "TERMINAL", "xterm" );
            environment.Set( "GUCEF_CONSOLE_LOGGER", "desktop" );

            CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
            ASSERT_FALSE( logManager.IsNativeConsoleLoggerEnabled() );
            logManager.SetNativeConsoleLoggerEnabled( true );
            logManager.SetNativeConsoleLoggerEnabled( true );
            ASSERT_TRUE( logManager.IsNativeConsoleLoggerEnabled() );
            logManager.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_CRITICAL, "console log window: switched on" );
            bool shown = false;
            for ( UInt32 waited=0; !shown && waited<10000; waited+=10 )
            {
                logManager.FlushLogs();
                CORE::CString content;
                shown = CORE::LoadTextFileAsString( outPath, content, false ) && content.HasSubstr( "console log window: switched on", true ) >= 0;
                if ( !shown )
                    MT::ThreadDelay( 10 );
            }
            ASSERT_TRUE( shown );

            // switched off it is no logger anymore: what is logged after that does not reach the window
            logManager.SetNativeConsoleLoggerEnabled( false );
            ASSERT_FALSE( logManager.IsNativeConsoleLoggerEnabled() );
            logManager.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_CRITICAL, "console log window: switched off" );
            logManager.FlushLogs();
            MT::ThreadDelay( 200 );
            CORE::CString content;
            CORE::LoadTextFileAsString( outPath, content, false );
            ASSERT_TRUE( content.HasSubstr( "console log window: switched off", true ) < 0 );
            logManager.SetNativeConsoleLoggerEnabled( false );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The log goes to a console window of its own when asked for, also from a terminal; the environment variable goes over it" )
        try
        {
            typedef CORE::CPlatformNativeConsoleLogger L;

            // without a wish for a window: the terminal first, a window of its own when there is no terminal
            ASSERT_TRUE( L::CONSOLEKIND_TERMINAL == L::ChooseConsoleKind( GUCEF_NULL, true, true, false ) );
            ASSERT_TRUE( L::CONSOLEKIND_DESKTOP_TERMINAL == L::ChooseConsoleKind( "", false, true, false ) );
            ASSERT_TRUE( L::CONSOLEKIND_NONE == L::ChooseConsoleKind( "auto", false, false, false ) );

            // a window of its own: also when started from a terminal, the terminal only when the desktop has no window to give
            ASSERT_TRUE( L::CONSOLEKIND_DESKTOP_TERMINAL == L::ChooseConsoleKind( GUCEF_NULL, true, true, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_TERMINAL == L::ChooseConsoleKind( GUCEF_NULL, true, false, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_NONE == L::ChooseConsoleKind( GUCEF_NULL, false, false, true ) );

            // GUCEF_CONSOLE_LOGGER decides when it is set
            ASSERT_TRUE( L::CONSOLEKIND_TERMINAL == L::ChooseConsoleKind( "terminal", true, true, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_NONE == L::ChooseConsoleKind( "terminal", false, true, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_DESKTOP_TERMINAL == L::ChooseConsoleKind( "desktop", true, false, false ) );
            ASSERT_TRUE( L::CONSOLEKIND_XTERM == L::ChooseConsoleKind( "xterm", true, true, false ) );
            ASSERT_TRUE( L::CONSOLEKIND_NONE == L::ChooseConsoleKind( "none", true, true, true ) );

            // the log manager keeps the choice; with the logger off it only remembers it
            CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
            const bool wasEnabled = logManager.IsNativeConsoleLoggerEnabled();
            const bool wasOwnWindow = logManager.IsNativeConsoleLoggerInOwnWindow();
            logManager.SetNativeConsoleLoggerEnabled( false );
            logManager.SetNativeConsoleLoggerInOwnWindow( !wasOwnWindow );
            ASSERT_TRUE( !wasOwnWindow == logManager.IsNativeConsoleLoggerInOwnWindow() && !logManager.IsNativeConsoleLoggerEnabled() );
            ASSERT_TRUE( GUCEF_NULL == logManager.GetNativeConsoleLogger() );
            logManager.SetNativeConsoleLoggerFormatAsConsoleUI( true );
            ASSERT_TRUE( logManager.IsNativeConsoleLoggerFormatAsConsoleUI() );
            logManager.SetNativeConsoleLoggerFormatAsConsoleUI( false );
            logManager.SetNativeConsoleLoggerInOwnWindow( wasOwnWindow );
            logManager.SetNativeConsoleLoggerEnabled( wasEnabled );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An application that ends with its console log window still on exits normally" )
        try
        {
            // the window is a fake terminal again, the test app itself is the application
            ASSERT_TRUE( WriteExecutable( CORE::CombinePath( binDir, "xterm" ), fakeXterm ) );
            CORE::DeleteFile( outPath );
            CEnvironmentScope environment;
            environment.Set( "PATH", searchPathWithFakes.C_String() );
            environment.Set( "GUCEF_FAKE_TERMINAL_OUT", outPath.C_String() );
            environment.Set( "TERMINAL", "xterm" );
            environment.Set( "GUCEF_CONSOLE_LOGGER", "desktop" );

            char exePath[ 4096 ];
            ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
            ASSERT_TRUE( exePathLength > 0 );
            exePath[ exePathLength ] = '\0';

            CORE::CProcess::TStringVector args;
            args.push_back( GUCEF_CORE_TESTAPP_EXIT_WITH_CONSOLE_LOG_WINDOW_ARG );
            CORE::CProcess application;
            ASSERT_TRUE( application.Launch( exePath, args, workDir ) );
            Int32 exitCode = -1;
            ASSERT_TRUE( application.WaitForExit( 60000, exitCode ) );

            // the window was on (else 2) and the exit went without an abort (128 + SIGABRT) or a crash
            ASSERT_TRUE( 0 == exitCode );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::Remove_Directory( workDir.C_String(), 1 );

    #else

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Desktop terminal console logger TESTS skipped: they are for Linux" );

    #endif /* GUCEF_LINUX_BUILD ? */
}

/*-------------------------------------------------------------------------*/

int
RunExitWithConsoleLogWindow( void )
{GUCEF_TRACE;

    // as an application with the console log window switched on in its settings, which it leaves on at the exit
    CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
    logManager.SetNativeConsoleLoggerInOwnWindow( true );
    logManager.SetNativeConsoleLoggerEnabled( true );
    if ( !logManager.IsNativeConsoleLoggerEnabled() )
        return 2;
    logManager.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_CRITICAL, "console log window: on until the exit" );
    logManager.FlushLogs();
    return 0;
}

/*-------------------------------------------------------------------------*/
