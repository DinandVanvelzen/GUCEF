/*
 *  gucefMULTIPLAY_DesktopNotifyTestApp: Application for testing the desktop match alert sink plugin of gucefMULTIPLAY
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

#include <sys/stat.h>
#include <stdio.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H
#include "notifysinkpluginDESKTOP_CDesktopMatchAlertSink.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H
#include "notifysinkpluginDESKTOP_CNotifySendLauncher.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H ? */

#include "TestDesktopNotifyCommon.h"
#include "TestDesktopNotify.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MULTIPLAYPLUGIN::DESKTOP;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

typedef INotifyLauncher::TArgumentVector TArgv;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
SameArgv( const TArgv& a, const TArgv& b )
{
    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
        if ( !( a[ i ] == b[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The arguments in front of the text, which a notification can never change */
static bool
HasFixedStart( const TArgv& argv )
{
    return argv.size() >= 5 && "notify-send" == argv[ 0 ] && "--app-name=Game" == argv[ 1 ] && "--urgency=normal" == argv[ 2 ] && "--" == argv[ 3 ];
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Repeat( const char* unit, UInt32 times )
{
    CORE::CString result;
    for ( UInt32 i=0; i<times; ++i )
        result += unit;
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
TestDirectory( const char* name )
{
    CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "desktopnotifytemp" ), name );
    CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** A file with a few bytes in it which is never run, with the given permission bits */
static bool
MakeFile( const CORE::CString& path, unsigned int mode )
{
    FILE* file = fopen( path.C_String(), "wb" );
    if ( GUCEF_NULL == file )
        return false;
    fputs( "not a program\n", file );
    fclose( file );
    return 0 == chmod( path.C_String(), (mode_t) mode );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDesktopNotifySinkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "notifysinkpluginDESKTOP: the desktop match alert sink" );

    GUCEF_TESTFW_TESTCASE( "Test: The sink interrupts and is called desktop" )
        try
        {
            CDesktopMatchAlertSink sink( GUCEF_NULL );
            ASSERT_TRUE( sink.IsInterrupting() );
            ASSERT_TRUE( "desktop" == sink.GetSinkName() );
            ASSERT_TRUE( "desktop" == CDesktopMatchAlertSink::SinkName );
            ASSERT_TRUE( 4 == CDesktopMatchAlertSink::DefaultMaxChildren && 4 == sink.GetMaxChildren() );
            ASSERT_TRUE( "Game" == sink.GetAppName() && sink.GetCategory().IsNULLOrEmpty() && 0 == sink.GetExpireTimeMs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The arguments of a turn are exactly notify-send, the options, -- and the text" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            TArgv argv;

            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "Friday game", "Bob", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Friday game", "It is your turn (after Bob)", GUCEF_NULL ) ) );

            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "Friday game", "", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Friday game", "It is your turn", GUCEF_NULL ) ) );

            // a match without a title
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "", "Bob", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Your turn", "It is your turn (after Bob)", GUCEF_NULL ) ) );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "  \n ", "", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Your turn", "It is your turn", GUCEF_NULL ) ) );

            // the options of the sink come before the --
            sink.SetAppName( "My Game" );
            ASSERT_TRUE( sink.SetCategory( "x-game.turn" ) );
            sink.SetExpireTimeMs( 5000 );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "M", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=My Game", "--urgency=normal", "--category=x-game.turn", "--expire-time=5000", "--", "T", "It is your turn (after M)", GUCEF_NULL ) ) );
            ASSERT_TRUE( "My Game" == sink.GetAppName() && "x-game.turn" == sink.GetCategory() && 5000 == sink.GetExpireTimeMs() );
            sink.SetExpireTimeMs( 0 );
            ASSERT_TRUE( sink.SetCategory( "" ) );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=My Game", "--urgency=normal", "--", "T", "It is your turn", GUCEF_NULL ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The application name, the category and the expire time are cleaned and bounded" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            TArgv argv;
            MULTIPLAY::CMatchAlert n = MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "", 1 );

            sink.SetAppName( "A\nB\x1b[0m" );
            ASSERT_TRUE( "A B [0m" == sink.GetAppName() );
            sink.SetAppName( "   " );
            ASSERT_TRUE( "Game" == sink.GetAppName() );
            sink.SetAppName( "" );
            ASSERT_TRUE( "Game" == sink.GetAppName() );
            sink.SetAppName( Repeat( "n", 100 ) );
            ASSERT_TRUE( 64 == sink.GetAppName().ByteSize() - 1 );
            sink.SetAppName( "--urgency=critical" );
            ASSERT_TRUE( sink.BuildArguments( n, argv ) );
            ASSERT_TRUE( 6 == argv.size() && "--app-name=--urgency=critical" == argv[ 1 ] && "--urgency=normal" == argv[ 2 ] && "--" == argv[ 3 ] );

            // a category that is not accepted leaves the one that was set
            ASSERT_TRUE( sink.SetCategory( "good.one" ) );
            ASSERT_FALSE( sink.SetCategory( "bad one" ) );
            ASSERT_FALSE( sink.SetCategory( "bad;one" ) );
            ASSERT_FALSE( sink.SetCategory( Repeat( "c", 65 ) ) );
            ASSERT_TRUE( "good.one" == sink.GetCategory() );

            sink.SetExpireTimeMs( 0xFFFFFFFF );
            ASSERT_TRUE( CDesktopMatchAlertSink::MaxExpireTimeMs == sink.GetExpireTimeMs() );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( n, argv ) );
            ASSERT_TRUE( "--expire-time=3600000" == argv[ 4 ] );

            sink.SetMaxChildren( 0 );
            ASSERT_TRUE( 1 == sink.GetMaxChildren() );
            sink.SetMaxChildren( 100000 );
            ASSERT_TRUE( 64 == sink.GetMaxChildren() );
            sink.SetMaxChildren( 7 );
            ASSERT_TRUE( 7 == sink.GetMaxChildren() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A title or a name that looks like an option stays in the text and is never taken for an option" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            const char* hostile[] = { "--help", "-u critical", "--hint=string:x:y", "-h", "--", "---", "--app-name=evil", "-t 1", "--urgency=critical", "--replace-id=1", "-", "--version", GUCEF_NULL };
            for ( int i=0; GUCEF_NULL != hostile[ i ]; ++i )
            {
                TArgv argv;
                ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, hostile[ i ], hostile[ i ], 1 ), argv ) );
                ASSERT_TRUE( HasFixedStart( argv ) && 6 == argv.size() );
                ASSERT_TRUE( hostile[ i ] == argv[ 4 ] );
                ASSERT_TRUE( CORE::CString( "It is your turn (after " ) + hostile[ i ] + ")" == argv[ 5 ] );
            }

            // the same through the other kinds
            for ( int kind = (int) MULTIPLAY::MATCHALERTKIND_YOUR_TURN; kind <= (int) MULTIPLAY::MATCHALERTKIND_FINISHED; ++kind )
            {
                TArgv argv;
                ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( (MULTIPLAY::TMatchAlertKind) kind, "--help", "-u critical", 3 ), argv ) );
                ASSERT_TRUE( HasFixedStart( argv ) );
                size_t firstSeparator = 0;
                while ( firstSeparator < argv.size() && "--" != argv[ firstSeparator ] )
                    ++firstSeparator;
                ASSERT_TRUE( 3 == firstSeparator );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Shell characters are text: one argument, as they were, nothing is interpreted" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            TArgv argv;
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "$(touch x); |&", "`id` && rm -rf ~ > /dev/null", 1 ), argv ) );
            ASSERT_TRUE( 6 == argv.size() );
            ASSERT_TRUE( "$(touch x); |&" == argv[ 4 ] );
            ASSERT_TRUE( "It is your turn (after `id` &amp;&amp; rm -rf ~ &gt; /dev/null)" == argv[ 5 ] );

            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "'; touch x; '\"\\ \\n ${HOME} *?[a-z]", "", 1 ), argv ) );
            ASSERT_TRUE( 6 == argv.size() && "'; touch x; '\"\\ \\n ${HOME} *?[a-z]" == argv[ 4 ] );

            // a line break cannot start a second argument or a second line of options
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "a\n--urgency=critical\r\n-t 1", "x\x1b]0;title\x07y", 1 ), argv ) );
            ASSERT_TRUE( 6 == argv.size() && "a --urgency=critical -t 1" == argv[ 4 ] );
            ASSERT_TRUE( !TestHasControl( argv[ 4 ] ) && !TestHasControl( argv[ 5 ] ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Title and name are cleaned, cut to length and the body is escaped" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            TArgv argv;
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, Repeat( "\xC3\xA9", 100 ).C_String(), Repeat( "<b>", 100 ).C_String(), 1 ), argv ) );
            ASSERT_TRUE( 6 == argv.size() );
            ASSERT_TRUE( TestIsValidUtf8( argv[ 4 ] ) && TestByteCount( argv[ 4 ] ) <= 64 && TestByteCount( argv[ 4 ] ) >= 60 );
            ASSERT_TRUE( TestIsValidUtf8( argv[ 5 ] ) && TestByteCount( argv[ 5 ] ) <= 160 && TestByteCount( argv[ 5 ] ) >= 150 );
            ASSERT_TRUE( argv[ 5 ].HasSubstr( "<" ) < 0 && argv[ 5 ].HasSubstr( ">" ) < 0 );
            ASSERT_TRUE( argv[ 5 ].HasSubstr( "\xE2\x80\xA6" ) >= 0 );

            // invalid UTF-8 in the title
            argv.clear();
            MULTIPLAY::CMatchAlert n = MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "", "", 1 );
            n.title = "Fri\xFF" "day game\xE2\x82";
            ASSERT_TRUE( sink.BuildArguments( n, argv ) );
            ASSERT_TRUE( 6 == argv.size() && "Friday game" == argv[ 4 ] );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every kind has its text: turn, coalesced with the count, digest, invitation without title or name, ended" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            TArgv argv;

            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_COALESCED, "", "", 3 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "3 games are waiting for you", GUCEF_NULL ) ) );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_COALESCED, "", "", 12 ), argv ) );
            ASSERT_TRUE( "12 games are waiting for you" == argv[ 4 ] );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_COALESCED, "", "", 1 ), argv ) );
            ASSERT_TRUE( "1 game is waiting for you" == argv[ 4 ] && 5 == argv.size() );

            // a coalesced or digest notification names no match even when the notification carries a title
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_COALESCED, "Secret title", "Alice", 2 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "2 games are waiting for you", GUCEF_NULL ) ) );

            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_DIGEST, "Secret title", "Alice", 5 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Daily digest", "5 games are waiting for you", GUCEF_NULL ) ) );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_DIGEST, "", "", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Daily digest", "1 game is waiting for you", GUCEF_NULL ) ) );

            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_INVITE, "Secret title", "Alice", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "You have a game invitation", GUCEF_NULL ) ) );
            for ( size_t i=0; i<argv.size(); ++i )
                ASSERT_TRUE( argv[ i ].HasSubstr( "Secret" ) < 0 && argv[ i ].HasSubstr( "Alice" ) < 0 );

            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_FINISHED, "Friday game", "Bob", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "A game has ended", "Friday game", GUCEF_NULL ) ) );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_FINISHED, "", "", 1 ), argv ) );
            ASSERT_TRUE( SameArgv( argv, MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "A game has ended", GUCEF_NULL ) ) );

            // a kind that does not exist shows nothing
            argv.clear();
            ASSERT_FALSE( sink.BuildArguments( MakeMatchAlert( (MULTIPLAY::TMatchAlertKind) 99, "T", "M", 1 ), argv ) );
            ASSERT_TRUE( argv.empty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A game can set the wording, and an invitation still gets no title and no name" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            CDesktopMatchAlertTexts texts;
            texts.yourTurnSummaryFallback = "Jij bent aan de beurt";
            texts.yourTurnBody = "Jouw beurt";
            texts.yourTurnBodyAfterMover = "Jouw beurt (na {mover}) in {title}";
            texts.coalescedSummary = "{count} spellen wachten";
            texts.inviteSummary = "Uitnodiging van {mover} voor {title}";
            texts.finishedSummary = "Spel afgelopen\nechtwaar";
            sink.SetTexts( texts );
            ASSERT_TRUE( "Jouw beurt" == sink.GetTexts().yourTurnBody );
            TArgv argv;
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "", "Bob", 1 ), argv ) );
            ASSERT_TRUE( 6 == argv.size() && "Jij bent aan de beurt" == argv[ 4 ] && "Jouw beurt (na Bob) in" == argv[ 5 ] );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "", 1 ), argv ) );
            ASSERT_TRUE( "Jouw beurt" == argv[ 5 ] );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_COALESCED, "", "", 4 ), argv ) );
            ASSERT_TRUE( 5 == argv.size() && "4 spellen wachten" == argv[ 4 ] );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_INVITE, "Secret title", "Alice", 1 ), argv ) );
            ASSERT_TRUE( 5 == argv.size() && "Uitnodiging van voor" == argv[ 4 ] );
            argv.clear();
            ASSERT_TRUE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_FINISHED, "", "", 1 ), argv ) );
            ASSERT_TRUE( 5 == argv.size() && "Spel afgelopen echtwaar" == argv[ 4 ] );

            // nothing to say, nothing to show
            texts.yourTurnSummaryFallback = "";
            sink.SetTexts( texts );
            argv.clear();
            ASSERT_FALSE( sink.BuildArguments( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "", "Bob", 1 ), argv ) );
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "", "Bob", 1 ) );
            ASSERT_TRUE( 0 == launcher.launched.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Deliver starts the launcher once with the arguments" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "Friday game", "Bob", 1 ) );
            ASSERT_TRUE( 1 == launcher.launched.size() && 1 == sink.GetLaunchedCount() && 0 == sink.GetDroppedCount() );
            ASSERT_TRUE( SameArgv( launcher.launched[ 0 ], MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Friday game", "It is your turn (after Bob)", GUCEF_NULL ) ) );
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_COALESCED, "", "", 3 ) );
            ASSERT_TRUE( 2 == launcher.launched.size() && 2 == sink.GetLaunchedCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: When notify-send is missing nothing is started and the sink says so once" )
        try
        {
            TESTFW::CLogTextCapture capture;
            CFakeNotifyLauncher launcher;
            launcher.available = false;
            CDesktopMatchAlertSink sink( &launcher );
            for ( int i=0; i<3; ++i )
                sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "SecretTitleXYZ", "SecretMoverXYZ", 1 ) );
            ASSERT_TRUE( 0 == launcher.launched.size() && 3 == launcher.availableChecks && 0 == sink.GetLaunchedCount() );
            ASSERT_TRUE( 1 == TestCountOccurrences( capture.GetText(), "notify-send is not available" ) );

            // it is there after all, and gone again: said again
            launcher.available = true;
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "", 1 ) );
            ASSERT_TRUE( 1 == launcher.launched.size() );
            launcher.available = false;
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "", 1 ) );
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "", 1 ) );
            ASSERT_TRUE( 1 == launcher.launched.size() );
            ASSERT_TRUE( 2 == TestCountOccurrences( capture.GetText(), "notify-send is not available" ) );
            // what the player plays is not written to the log
            ASSERT_FALSE( capture.Contains( "SecretTitleXYZ" ) || capture.Contains( "SecretMoverXYZ" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No more programs run at the same time than the limit, the rest is dropped and counted" )
        try
        {
            TESTFW::CLogTextCapture capture;
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            MULTIPLAY::CMatchAlert n = MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "SecretTitleXYZ", "Bob", 1 );
            launcher.running = 3;
            sink.Deliver( n );
            ASSERT_TRUE( 1 == launcher.launched.size() && 0 == sink.GetDroppedCount() );
            launcher.running = 4;
            sink.Deliver( n );
            sink.Deliver( n );
            ASSERT_TRUE( 1 == launcher.launched.size() && 2 == sink.GetDroppedCount() && 1 == sink.GetLaunchedCount() );
            ASSERT_TRUE( capture.Contains( "dropped" ) );
            ASSERT_FALSE( capture.Contains( "SecretTitleXYZ" ) );
            launcher.running = 9;
            sink.Deliver( n );
            ASSERT_TRUE( 1 == launcher.launched.size() && 3 == sink.GetDroppedCount() );
            sink.SetMaxChildren( 10 );
            sink.Deliver( n );
            ASSERT_TRUE( 2 == launcher.launched.size() );
            sink.SetMaxChildren( 1 );
            launcher.running = 1;
            sink.Deliver( n );
            ASSERT_TRUE( 2 == launcher.launched.size() && 4 == sink.GetDroppedCount() );
            launcher.running = 0;
            sink.Deliver( n );
            ASSERT_TRUE( 3 == launcher.launched.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A launcher that fails or throws, or no launcher at all, never makes Deliver throw" )
        try
        {
            TESTFW::CLogTextCapture capture;
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            MULTIPLAY::CMatchAlert n = MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "Bob", 1 );

            launcher.launchResult = false;
            sink.Deliver( n );
            ASSERT_TRUE( 1 == launcher.launched.size() && 0 == sink.GetLaunchedCount() );
            ASSERT_TRUE( capture.Contains( "could not be started" ) );
            launcher.launchResult = true;

            launcher.throwOnLaunch = true;
            sink.Deliver( n );
            ASSERT_TRUE( 2 == launcher.launched.size() && 0 == sink.GetLaunchedCount() );
            launcher.throwOnLaunch = false;

            launcher.throwOnAvailable = true;
            sink.Deliver( n );
            launcher.throwOnAvailable = false;
            ASSERT_TRUE( 2 == launcher.launched.size() );

            // and it goes on afterwards
            sink.Deliver( n );
            ASSERT_TRUE( 3 == launcher.launched.size() && 1 == sink.GetLaunchedCount() );

            sink.SetLauncher( GUCEF_NULL );
            sink.Deliver( n );
            sink.Deliver( n );
            ASSERT_TRUE( 3 == launcher.launched.size() );
            CDesktopMatchAlertSink nobody( GUCEF_NULL );
            nobody.Deliver( n );
            nobody.Clear( "x" );
            ASSERT_TRUE( 0 == nobody.GetLaunchedCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Clear is harmless: it starts nothing, asks nothing and takes nothing back" )
        try
        {
            CFakeNotifyLauncher launcher;
            CDesktopMatchAlertSink sink( &launcher );
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "Bob", 1 ) );
            sink.Clear( "match-1" );
            sink.Clear( "" );
            sink.Clear( "--help" );
            sink.Clear( "unknown match" );
            ASSERT_TRUE( 1 == launcher.launched.size() && 1 == launcher.availableChecks && 1 == sink.GetLaunchedCount() );
            sink.Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "Bob", 1 ) );
            ASSERT_TRUE( 2 == launcher.launched.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The launcher finds an executable file in the search path and nothing else" )
        try
        {
            CORE::CString root = TestDirectory( "search" );
            CORE::CString withTool = CORE::CombinePath( root, "withtool" );
            CORE::CString notExecutable = CORE::CombinePath( root, "notexecutable" );
            CORE::CString toolIsDir = CORE::CombinePath( root, "toolisdir" );
            CORE::CString emptyDir = CORE::CombinePath( root, "empty" );
            CORE::CString sub = CORE::CombinePath( withTool, "sub" );
            CORE::CreateDirs( withTool );
            CORE::CreateDirs( notExecutable );
            CORE::CreateDirs( CORE::CombinePath( toolIsDir, "notify-send" ) );
            CORE::CreateDirs( emptyDir );
            CORE::CreateDirs( sub );
            ASSERT_TRUE( MakeFile( CORE::CombinePath( withTool, "notify-send" ), 0755 ) );
            ASSERT_TRUE( MakeFile( CORE::CombinePath( notExecutable, "notify-send" ), 0644 ) );
            ASSERT_TRUE( MakeFile( CORE::CombinePath( sub, "notify-send" ), 0755 ) );
            CORE::CString expected = CORE::CombinePath( withTool, "notify-send" );
            CORE::CString found;

            ASSERT_TRUE( "notify-send" == CNotifySendLauncher::GetToolName() );
            ASSERT_TRUE( CNotifySendLauncher::FindExecutable( "notify-send", withTool, found ) && expected == found );
            // the first directory with the file that can be run wins
            found.Clear();
            ASSERT_TRUE( CNotifySendLauncher::FindExecutable( "notify-send", notExecutable + ":" + emptyDir + ":" + toolIsDir + ":/does/not/exist:" + withTool, found ) && expected == found );
            // not executable, a directory, or not there
            found.Clear();
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "notify-send", notExecutable, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "notify-send", toolIsDir, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "notify-send", emptyDir, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "notify-send", "", found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "notify-send", ":::", found ) );
            ASSERT_TRUE( found.IsNULLOrEmpty() );
            // a name with a path in it is no name
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "sub/notify-send", withTool, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "../withtool/notify-send", emptyDir, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( withTool + "/notify-send", withTool, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "", withTool, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( ".", withTool, found ) );
            ASSERT_FALSE( CNotifySendLauncher::FindExecutable( "..", root, found ) );

            // the launcher with such a search path
            CNotifySendLauncher launcher;
            launcher.SetSearchPath( withTool );
            ASSERT_TRUE( launcher.IsAvailable() );
            launcher.SetSearchPath( emptyDir );
            ASSERT_FALSE( launcher.IsAvailable() );
            launcher.SetSearchPath( notExecutable );
            ASSERT_FALSE( launcher.IsAvailable() );
            launcher.SetSearchPath( withTool );
            ASSERT_TRUE( launcher.IsAvailable() );
            ASSERT_TRUE( 0 == launcher.GetRunningCount() );
            CORE::DeleteDir( root, true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The launcher refuses what is not a call of notify-send, without starting anything" )
        try
        {
            CORE::CString root = TestDirectory( "refuse" );
            CNotifySendLauncher launcher;
            launcher.SetSearchPath( root );                          // a directory without the program
            ASSERT_FALSE( launcher.IsAvailable() );
            ASSERT_FALSE( launcher.Launch( TArgv() ) );
            ASSERT_FALSE( launcher.Launch( MakeArgv( "rm", "-rf", "/", GUCEF_NULL ) ) );
            ASSERT_FALSE( launcher.Launch( MakeArgv( "/usr/bin/notify-send", "--", "x", GUCEF_NULL ) ) );
            ASSERT_FALSE( launcher.Launch( MakeArgv( "", "--", "x", GUCEF_NULL ) ) );
            // the right name but the program is not there
            ASSERT_FALSE( launcher.Launch( MakeArgv( "notify-send", "--", "x", GUCEF_NULL ) ) );
            ASSERT_TRUE( 0 == launcher.GetRunningCount() );
            CORE::DeleteDir( root, true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The plugin puts desktop in the factory of the match alert sinks, and takes it out when it unloads" )
        try
        {
            MULTIPLAY::CMatchAlertSinkFactory& factory = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchAlertSinkFactory();
            ASSERT_TRUE( factory.Create( "desktop" ).IsNULL() );
            ASSERT_TRUE( LoadTestPlugin( "notifysinkpluginDESKTOP", "desktopsink" ) );
            {
                MULTIPLAY::CIMatchAlertSinkPtr product = factory.Create( "desktop" );
                ASSERT_FALSE( product.IsNULL() );
                if ( !product.IsNULL() )
                {
                    MULTIPLAY::CIMatchAlertSink* sink = product.GetPointerAlways();
                    ASSERT_TRUE( "desktop" == sink->GetSinkName() && sink->IsInterrupting() );
                    CDesktopMatchAlertSink* desktop = static_cast< CDesktopMatchAlertSink* >( sink );
                    ASSERT_TRUE( 4 == desktop->GetMaxChildren() && "Game" == desktop->GetAppName() );

                    // a sink from the factory gets the launcher of the application
                    CFakeNotifyLauncher launcher;
                    desktop->SetLauncher( &launcher );
                    sink->Deliver( MakeMatchAlert( MULTIPLAY::MATCHALERTKIND_YOUR_TURN, "T", "Bob", 1 ) );
                    ASSERT_TRUE( 1 == launcher.launched.size() );
                    desktop->SetLauncher( GUCEF_NULL );
                }
                ASSERT_TRUE( factory.Create( "mail" ).IsNULL() );
            }
            CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "desktopsink" );
            ASSERT_TRUE( factory.Create( "desktop" ).IsNULL() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
