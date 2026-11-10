/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H
#include "authpluginSTEAM_CSteamLocalDetector.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H
#include "authpluginSTEAM_CSystemSteamHostView.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H
#include "authpluginSTEAM_CSteamKeyValues.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFakeSteamHost.h"
#include "TestSteamDetector.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A directory for a test, below the working directory of the test app, emptied first */
static CORE::CString
MakeTestDirectory( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "authtemp" ), name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** The names of the files and directories in a directory */
static CORE::CString::StringVector
ListDirectory( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( dir ) )
    {
        do
        {
            names.push_back( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return names;
}

/*-------------------------------------------------------------------------*/

static bool
HasName( const CORE::CString::StringVector& names, const CORE::CString& name )
{GUCEF_TRACE;

    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt64 IdA = 76561197960265728ULL + 12345678ULL;     /**< account id 12345678 */
static const UInt64 IdB = 76561197960265728ULL + 87654321ULL;     /**< account id 87654321 */
static const UInt64 IdC = 76561197960265728ULL + 1000ULL;

static CORE::CString
Repeated( char c, UInt32 count )
{
    char* buffer = GUCEF_NEW char[ count + 1 ];
    memset( buffer, c, count );
    buffer[ count ] = '\0';
    CORE::CString text( buffer, count );
    delete [] buffer;
    return text;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
MakeUserBlock( UInt64 steamId, const CORE::CString& account, const CORE::CString& persona, bool mostRecent, UInt64 timestamp )
{
    return "\t\"" + CORE::UInt64ToString( steamId ) + "\"\n\t{\n"
           "\t\t\"AccountName\"\t\t\"" + account + "\"\n"
           "\t\t\"PersonaName\"\t\t\"" + persona + "\"\n"
           "\t\t\"RememberPassword\"\t\t\"1\"\n"
           "\t\t\"MostRecent\"\t\t\"" + ( mostRecent ? "1" : "0" ) + "\"\n"
           "\t\t\"Timestamp\"\t\t\"" + CORE::UInt64ToString( timestamp ) + "\"\n\t}\n";
}

static CORE::CString
MakeLoginUsers( const CORE::CString& blocks )
{
    return "\"users\"\n{\n" + blocks + "}\n";
}

static CORE::CString
MakeRegistryVdf( UInt32 pid, UInt32 activeUser )
{
    return "\"Registry\"\n{\n\t\"HKCU\"\n\t{\n\t\t\"Software\"\n\t\t{\n\t\t\t\"Valve\"\n\t\t\t{\n\t\t\t\t\"Steam\"\n\t\t\t\t{\n"
           "\t\t\t\t\t\"language\"\t\t\"english\"\n"
           "\t\t\t\t\t\"ActiveProcess\"\n\t\t\t\t\t{\n"
           "\t\t\t\t\t\t\"pid\"\t\t\"" + CORE::UInt32ToString( pid ) + "\"\n"
           "\t\t\t\t\t\t\"ActiveUser\"\t\t\"" + CORE::UInt32ToString( activeUser ) + "\"\n"
           "\t\t\t\t\t}\n\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}\n";
}

/** A Linux machine with a client in ~/.steam/steam that has had two users */
static void
MakeLinuxClient( CFakeHostView& view )
{
    view.AddFile( "/home/test/.steam/steam/config/loginusers.vdf",
                  MakeLoginUsers( MakeUserBlock( IdA, "alice", "Alice P", false, 1700000000ULL ) + MakeUserBlock( IdB, "bob", "Bob Q", true, 1600000000ULL ) ) );
}

/*-------------------------------------------------------------------------*/

void
PerformSteamDetectorTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SteamDetector TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "SteamDetector" );

    GUCEF_TESTFW_TESTCASE( "Test: Key values text: blocks, quoted and bare tokens, comments, escapes, conditionals and a byte order mark" )
        try
        {
            AUTHPLUGIN::STEAM::CSteamKeyValues kv;
            const CORE::CString text = CORE::CString( "\xEF\xBB\xBF" ) +
                "// a comment\n"
                "\"Root\" // more comment\n"
                "{\n"
                "    \"quoted key\"   \"quoted value\"\n"
                "    bare   value\n"
                "    \"escapes\"  \"a\\tb\\nc\\\\d\\\"e\"\n"
                "    \"cond\"  \"x\" [$WIN32]\n"
                "    \"empty\"  \"\"\n"
                "    \"Inner\" { \"deep\" \"1\" \"Deeper\" { \"deepest\" \"2\" } }\n"
                "}\n"
                "\"Second\" { \"k\" \"v\" }\n";
            ASSERT_TRUE( kv.Parse( text ) );

            AUTHPLUGIN::STEAM::CSteamKeyValues::TIndex root = kv.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "Root" );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex != root && kv.IsBlock( root ) );
            ASSERT_TRUE( "quoted value" == kv.GetChildValue( root, "quoted key" ) );
            ASSERT_TRUE( "value" == kv.GetChildValue( root, "bare" ) );
            ASSERT_TRUE( "a\tb\nc\\d\"e" == kv.GetChildValue( root, "escapes" ) );
            ASSERT_TRUE( "x" == kv.GetChildValue( root, "cond" ) );
            ASSERT_TRUE( kv.GetChildValue( root, "empty", "fallback" ).IsNULLOrEmpty() );
            AUTHPLUGIN::STEAM::CSteamKeyValues::TIndex inner = kv.FindChild( root, "Inner" );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex != inner );
            ASSERT_TRUE( "1" == kv.GetChildValue( inner, "deep" ) );
            CORE::CString::StringVector path;
            path.push_back( "Inner" );
            path.push_back( "Deeper" );
            AUTHPLUGIN::STEAM::CSteamKeyValues::TIndex deeper = kv.FindBlock( root, path );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex != deeper && "2" == kv.GetChildValue( deeper, "deepest" ) );
            ASSERT_TRUE( "v" == kv.GetChildValue( kv.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "Second" ), "k" ) );

            AUTHPLUGIN::STEAM::CSteamKeyValues::TIndexVector children;
            kv.GetChildren( root, children );
            ASSERT_TRUE( 6 == children.size() && "quoted key" == kv.GetKey( children[ 0 ] ) && !kv.IsBlock( children[ 0 ] ) );

            // a copy has the same entries, and the original can be read again
            AUTHPLUGIN::STEAM::CSteamKeyValues copy( kv );
            ASSERT_TRUE( copy.GetEntryCount() == kv.GetEntryCount() && "quoted value" == copy.GetChildValue( copy.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "Root" ), "quoted key" ) );
            ASSERT_TRUE( kv.Parse( "\"a\" \"b\"" ) && 1 == kv.GetEntryCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Key values text: keys are matched without regard to case, a missing key or a block gives the default" )
        try
        {
            AUTHPLUGIN::STEAM::CSteamKeyValues kv;
            ASSERT_TRUE( kv.Parse( "\"UsErS\" { \"AccountName\" \"x\" \"Block\" { \"a\" \"b\" } }" ) );
            AUTHPLUGIN::STEAM::CSteamKeyValues::TIndex users = kv.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "users" );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex != users );
            ASSERT_TRUE( "x" == kv.GetChildValue( users, "ACCOUNTNAME" ) );
            ASSERT_TRUE( "d" == kv.GetChildValue( users, "missing", "d" ) );
            ASSERT_TRUE( "d" == kv.GetChildValue( users, "block", "d" ) );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex == kv.FindChild( users, "nothing" ) );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex == kv.FindChild( 999999, "x" ) );
            CORE::CString::StringVector path;
            path.push_back( "AccountName" );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex == kv.FindBlock( users, path ) );   // a value is not a block
            ASSERT_TRUE( kv.GetKey( 999999 ).IsNULLOrEmpty() && kv.GetValue( 999999 ).IsNULLOrEmpty() && !kv.IsBlock( 999999 ) );
            ASSERT_TRUE( kv.Parse( "" ) && 0 == kv.GetEntryCount() );
            ASSERT_TRUE( kv.Parse( "   \n\t // only a comment" ) && 0 == kv.GetEntryCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Key values text: malformed text is refused and what was read before the fault stays" )
        try
        {
            // a block that is not closed
            AUTHPLUGIN::STEAM::CSteamKeyValues kv;
            ASSERT_FALSE( kv.Parse( "\"users\" { \"a\" \"1\" \"b\" \"2\"" ) );
            ASSERT_TRUE( "1" == kv.GetChildValue( kv.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "users" ), "a" ) );
            ASSERT_TRUE( "2" == kv.GetChildValue( kv.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "users" ), "b" ) );

            // a closing brace too many
            ASSERT_FALSE( kv.Parse( "\"a\" \"1\" } \"b\" \"2\"" ) );
            ASSERT_TRUE( "1" == kv.GetChildValue( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "a" ) && AUTHPLUGIN::STEAM::CSteamKeyValues::NoIndex == kv.FindChild( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "b" ) );

            // a key without a value, a value that is a closing brace
            ASSERT_FALSE( kv.Parse( "\"a\"" ) );
            ASSERT_FALSE( kv.Parse( "\"a\" }" ) );
            ASSERT_FALSE( kv.Parse( "{ \"a\" \"b\" }" ) );
            ASSERT_FALSE( kv.Parse( "{{{{{{" ) );
            ASSERT_FALSE( kv.Parse( "}" ) );

            // a string that is not closed
            ASSERT_FALSE( kv.Parse( "\"a\" \"1\" \"b\" \"never closed" ) );
            ASSERT_TRUE( "1" == kv.GetChildValue( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "a" ) );
            ASSERT_FALSE( kv.Parse( "\"never closed" ) );

            // a cut in the middle of an escape
            ASSERT_FALSE( kv.Parse( "\"a\" \"b\\" ) );

            // binary nonsense is read as whatever it is, without a crash
            CORE::CString noise;
            for ( UInt32 i=1; i<256; ++i )
                noise += CORE::CString( (char)( 'A' + ( i * 7 ) % 26 ) ) + ( 0 == i % 5 ? " " : "" );
            kv.Parse( noise );
            kv.Parse( "\x01\x02\x03\xFF\xFE\"\"\"\"" );
            ASSERT_TRUE( kv.GetEntryCount() < 100 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Key values text: huge and hostile input stays inside the limits and is quick" )
        try
        {
            const UInt64 start = MT::PrecisionTickCount();
            AUTHPLUGIN::STEAM::CSteamKeyValues kv;

            // more entries than the limit: refused, and no more than the limit is kept
            CORE::CString many;
            for ( UInt32 i=0; i<20000; ++i )
                many += "\"k\" \"v\"\n";
            ASSERT_FALSE( kv.Parse( many ) );
            ASSERT_TRUE( kv.GetEntryCount() <= AUTHPLUGIN::STEAM::CSteamKeyValues::MaxEntries + 1 );

            // blocks nested deeper than the limit
            CORE::CString deep;
            for ( UInt32 i=0; i<200; ++i )
                deep += "\"b\" {";
            ASSERT_FALSE( kv.Parse( deep ) );
            ASSERT_TRUE( kv.GetEntryCount() <= AUTHPLUGIN::STEAM::CSteamKeyValues::MaxDepth + 2 );

            // a token longer than the limit, quoted and bare
            ASSERT_FALSE( kv.Parse( "\"" + Repeated( 'a', 10000 ) + "\" \"v\"" ) );
            ASSERT_FALSE( kv.Parse( Repeated( 'a', 10000 ) + " v" ) );
            ASSERT_FALSE( kv.Parse( "\"k\" \"" + Repeated( 'a', 10000 ) + "\"" ) );

            // text over the size limit: whitespace and one real entry in front
            CORE::CString big = Repeated( ' ', 5 * 1024 * 1024 );
            big = "\"first\" \"1\"\n" + big + "\"late\" \"2\"";
            ASSERT_FALSE( kv.Parse( big ) );
            ASSERT_TRUE( "1" == kv.GetChildValue( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "first" ) && "2" != kv.GetChildValue( AUTHPLUGIN::STEAM::CSteamKeyValues::RootIndex, "late" ) );

            // a megabyte of braces
            ASSERT_FALSE( kv.Parse( Repeated( '{', 1024 * 1024 ) ) );
            ASSERT_FALSE( kv.Parse( Repeated( '}', 1024 * 1024 ) ) );

            // and as a loginusers file
            AUTHPLUGIN::STEAM::TSteamLocalUserVector users;
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( "\"users\" {" + many, users ) && users.size() > 0 );

            // all of it quickly
            const UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
            ASSERT_TRUE( elapsedMs < 20000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The users of loginusers.vdf: ids, names, the most recent flag and the time of the last login" )
        try
        {
            AUTHPLUGIN::STEAM::TSteamLocalUserVector users;
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( MakeLoginUsers( MakeUserBlock( IdA, "alice", "Alice P", false, 1700000000ULL ) + MakeUserBlock( IdB, "bob", "Bob Q", true, 1600000000ULL ) ), users ) );
            ASSERT_TRUE( 2 == users.size() );
            ASSERT_TRUE( IdA == users[ 0 ].steamId && "alice" == users[ 0 ].accountName && "Alice P" == users[ 0 ].personaName && !users[ 0 ].mostRecent && 1700000000ULL == users[ 0 ].timestamp );
            ASSERT_TRUE( IdB == users[ 1 ].steamId && "bob" == users[ 1 ].accountName && "Bob Q" == users[ 1 ].personaName && users[ 1 ].mostRecent && 1600000000ULL == users[ 1 ].timestamp );

            // keys in other cases, the same id twice (the later wins), a user without names or flags
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( "\"USERS\" { \"" + CORE::UInt64ToString( IdA ) + "\" { \"accountname\" \"one\" } \"" + CORE::UInt64ToString( IdA ) + "\" { \"ACCOUNTNAME\" \"two\" \"mostrecent\" \"1\" } }", users ) );
            ASSERT_TRUE( 1 == users.size() && "two" == users[ 0 ].accountName && users[ 0 ].personaName.IsNULLOrEmpty() && users[ 0 ].mostRecent && 0 == users[ 0 ].timestamp );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The users of loginusers.vdf: what is not a SteamID64 is left out and names are made safe" )
        try
        {
            AUTHPLUGIN::STEAM::TSteamLocalUserVector users;
            const CORE::CString badIds =
                "\"users\" {\n"
                "  \"notanumber\" { \"AccountName\" \"x1\" }\n"
                "  \"12345\" { \"AccountName\" \"x2\" }\n"
                "  \"76561197960265727\" { \"AccountName\" \"x3\" }\n"      // one below the first individual account
                "  \"76561202255233024\" { \"AccountName\" \"x4\" }\n"      // one past the last individual account
                "  \"99999999999999999999999\" { \"AccountName\" \"x5\" }\n"
                "  \"-76561197960265729\" { \"AccountName\" \"x6\" }\n"
                "  \"7656119796026572a\" { \"AccountName\" \"x7\" }\n"
                "  \"76561197960265728\" { \"AccountName\" \"first\" }\n"   // the first individual account
                "  \"76561202255233023\" { \"AccountName\" \"last\" }\n"    // the last
                "  \"76561197960265729\" \"a value, not a user\"\n"
                "}\n";
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( badIds, users ) );
            ASSERT_TRUE( 2 == users.size() && "first" == users[ 0 ].accountName && "last" == users[ 1 ].accountName );

            // control characters go, the name is trimmed, a long name is cut off in whole characters
            const CORE::CString longName = Repeated( 'n', 300 );
            AUTHPLUGIN::STEAM::TSteamLocalUserVector named;
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( MakeLoginUsers( MakeUserBlock( IdA, "a\x01" "b\x7F", "   Pers\tona\n  ", false, 1 ) + MakeUserBlock( IdB, "b", longName, false, 1 ) ), named ) );
            ASSERT_TRUE( 2 == named.size() && "ab" == named[ 0 ].accountName && "Persona" == named[ 0 ].personaName );
            ASSERT_TRUE( 64 == strlen( named[ 1 ].personaName.C_String() ) );
            ASSERT_TRUE( "Dinand" == AUTHPLUGIN::STEAM::CSteamLocalDetector::MakeSafeName( "  Dinand  ", 64 ) );
            ASSERT_TRUE( "Din" == AUTHPLUGIN::STEAM::CSteamLocalDetector::MakeSafeName( "Dinand", 3 ) );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::MakeSafeName( "\x01\x02\x03", 64 ).IsNULLOrEmpty() );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::MakeSafeName( "", 64 ).IsNULLOrEmpty() );
            // multi byte characters stay whole: three characters of two bytes each
            ASSERT_TRUE( "\xC3\xBC\xC3\xBC\xC3\xBC" == AUTHPLUGIN::STEAM::CSteamLocalDetector::MakeSafeName( "\xC3\xBC\xC3\xBC\xC3\xBC\xC3\xBC\xC3\xBC", 3 ) );

            // not a loginusers file: no users block
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( "\"other\" { }", users ) && users.size() > 0 );
            ASSERT_TRUE( users.empty() );
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( "", users ) );
            // a file cut short: what was read stays
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseLoginUsers( "\"users\" { \"" + CORE::UInt64ToString( IdA ) + "\" { \"AccountName\" \"cut\"", users ) );
            ASSERT_TRUE( 1 == users.size() && "cut" == users[ 0 ].accountName );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The user to name is the active one, else the most recent, else the latest" )
        try
        {
            AUTHPLUGIN::STEAM::TSteamLocalUserVector users;
            AUTHPLUGIN::STEAM::CSteamLocalUser user;
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 0, user ) );

            AUTHPLUGIN::STEAM::CSteamLocalUser a, b, c;
            a.steamId = IdA; a.personaName = "A"; a.mostRecent = false; a.timestamp = 300;
            b.steamId = IdB; b.personaName = "B"; b.mostRecent = true;  b.timestamp = 100;
            c.steamId = IdC; c.personaName = "C"; c.mostRecent = false; c.timestamp = 200;
            users.push_back( a ); users.push_back( b ); users.push_back( c );

            // the marked one beats the later one
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 0, user ) && "B" == user.personaName );
            // the active account beats the marked one
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 12345678, user ) && "A" == user.personaName );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 1000, user ) && "C" == user.personaName );
            // an active account the file does not know is no choice
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 55, user ) && "B" == user.personaName );
            // several marked: the latest of them
            users[ 0 ].mostRecent = true;
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 0, user ) && "A" == user.personaName );
            // none marked: the latest
            users[ 0 ].mostRecent = false; users[ 1 ].mostRecent = false;
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 0, user ) && "A" == user.personaName );
            // the same time: the first
            users[ 0 ].timestamp = 100; users[ 1 ].timestamp = 100; users[ 2 ].timestamp = 100;
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ChooseUser( users, 0, user ) && "A" == user.personaName );

            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::IsIndividualSteamId( 76561197960265728ULL ) && AUTHPLUGIN::STEAM::CSteamLocalDetector::IsIndividualSteamId( 76561202255233023ULL ) );
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::IsIndividualSteamId( 0 ) || AUTHPLUGIN::STEAM::CSteamLocalDetector::IsIndividualSteamId( 76561197960265727ULL ) || AUTHPLUGIN::STEAM::CSteamLocalDetector::IsIndividualSteamId( 76561202255233024ULL ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The active process of registry.vdf gives the process id and the active user" )
        try
        {
            UInt32 pid = 1, user = 1;
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseActiveProcess( MakeRegistryVdf( 4242, 12345678 ), pid, user ) && 4242 == pid && 12345678 == user );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseActiveProcess( MakeRegistryVdf( 0, 0 ), pid, user ) && 0 == pid && 0 == user );
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseActiveProcess( "\"Registry\" { }", pid, user ) );
            ASSERT_TRUE( 0 == pid && 0 == user );
            ASSERT_FALSE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseActiveProcess( "", pid, user ) );
            // a value that is not a number, a number too big for a process id
            ASSERT_TRUE( AUTHPLUGIN::STEAM::CSteamLocalDetector::ParseActiveProcess( "\"Registry\" { \"HKCU\" { \"Software\" { \"Valve\" { \"Steam\" { \"ActiveProcess\" { \"pid\" \"abc\" \"ActiveUser\" \"99999999999\" } } } } } }", pid, user ) );
            ASSERT_TRUE( 0 == pid && 0 == user );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without Steam nothing is detected, on every platform" )
        try
        {
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            info.installed = true;
            info.hasUser = true;
            info.user.personaName = "stale";
            const AUTHPLUGIN::STEAM::TSteamHostPlatform platforms[ 4 ] = { AUTHPLUGIN::STEAM::STEAMHOST_LINUX, AUTHPLUGIN::STEAM::STEAMHOST_WINDOWS, AUTHPLUGIN::STEAM::STEAMHOST_MACOS, AUTHPLUGIN::STEAM::STEAMHOST_OTHER };
            for ( UInt32 p=0; p<4; ++p )
            {
                CFakeHostView view( platforms[ p ] );
                view.env[ "ProgramFiles(x86)" ] = "C:/Program Files (x86)";
                // folders of the same names without what a client has in them are not a client
                view.AddDir( "/home/test/.steam/steam" );
                view.AddDir( "/home/test/.local/share/Steam" );
                view.AddDir( "/home/test/Library/Application Support/Steam" );
                view.AddDir( "C:/Program Files (x86)/Steam" );
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_FALSE( detector.Detect( info ) );
                ASSERT_FALSE( info.installed || info.running || info.hasUser );
                ASSERT_TRUE( info.user.personaName.IsNULLOrEmpty() && info.installPath.IsNULLOrEmpty() );
            }

            // no home, no registry
            CFakeHostView noHome( AUTHPLUGIN::STEAM::STEAMHOST_LINUX, "" );
            AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &noHome );
            ASSERT_FALSE( detector.Detect( info ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Linux: an installed client and the user that logged in last" )
        try
        {
            CFakeHostView view;
            MakeLinuxClient( view );
            AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            ASSERT_TRUE( detector.Detect( info ) );
            ASSERT_TRUE( info.installed && !info.running && "/home/test/.steam/steam" == info.installPath );
            ASSERT_TRUE( info.hasUser && IdB == info.user.steamId && "Bob Q" == info.user.personaName && "bob" == info.user.accountName );

            CORE::CString::StringVector folders;
            detector.GetCandidateInstallFolders( folders );
            ASSERT_TRUE( 3 == folders.size() && "/home/test/.steam/steam" == folders[ 0 ] && "/home/test/.local/share/Steam" == folders[ 1 ] && "/home/test/.var/app/com.valvesoftware.Steam/.local/share/Steam" == folders[ 2 ] );

            // the same view gives the same answer again, nothing is kept or changed
            AUTHPLUGIN::STEAM::CSteamLocalInfo again;
            ASSERT_TRUE( detector.Detect( again ) && again.user.steamId == info.user.steamId );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Linux: the client is running when the process of steam.pid is there and is a Steam process" )
        try
        {
            CFakeHostView view;
            MakeLinuxClient( view );
            view.AddFile( "/home/test/.steam/steam.pid", "4242\n" );
            AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;

            // no such process: a pid file of a client that is gone
            ASSERT_TRUE( detector.Detect( info ) && !info.running );

            view.processes[ "4242" ] = "steam";
            ASSERT_TRUE( detector.Detect( info ) && info.running && "Bob Q" == info.user.personaName );

            // the id of another process is not the client
            view.processes[ "4242" ] = "bash";
            ASSERT_TRUE( detector.Detect( info ) && !info.running );

            // garbage in the pid file
            view.processes[ "4242" ] = "steam";
            const char* garbage[] = { "", "abc", "-1", "4242abc", "99999999999999999999", "0", "\n", "4242 4243" };
            for ( UInt32 i=0; i<sizeof( garbage ) / sizeof( garbage[ 0 ] ); ++i )
            {
                view.files[ "/home/test/.steam/steam.pid" ] = garbage[ i ];
                ASSERT_TRUE( detector.Detect( info ) );
                ASSERT_FALSE( info.running );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Linux: registry.vdf says the process and the user that is logged in while the client runs" )
        try
        {
            CFakeHostView view;
            MakeLinuxClient( view );
            view.AddFile( "/home/test/.steam/registry.vdf", MakeRegistryVdf( 5151, 12345678 ) );
            AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;

            // not running: the one that logged in last, the active user is ignored
            ASSERT_TRUE( detector.Detect( info ) && !info.running && IdB == info.user.steamId );

            // running: the one that is logged in now, even if another logged in last
            view.processes[ "5151" ] = "steam";
            ASSERT_TRUE( detector.Detect( info ) && info.running && info.hasUser && IdA == info.user.steamId && "Alice P" == info.user.personaName );

            // logged in as somebody the file does not know: back to the last one
            view.files[ "/home/test/.steam/registry.vdf" ] = MakeRegistryVdf( 5151, 4 );
            ASSERT_TRUE( detector.Detect( info ) && info.running && IdB == info.user.steamId );

            // logged out (user 0)
            view.files[ "/home/test/.steam/registry.vdf" ] = MakeRegistryVdf( 5151, 0 );
            ASSERT_TRUE( detector.Detect( info ) && info.running && IdB == info.user.steamId );

            // the registry of the install folder is read as well
            view.files.erase( "/home/test/.steam/registry.vdf" );
            view.AddFile( "/home/test/.steam/steam/registry.vdf", MakeRegistryVdf( 5151, 12345678 ) );
            ASSERT_TRUE( detector.Detect( info ) && info.running && IdA == info.user.steamId );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Linux: the other places of a client, an empty folder is skipped and the Flatpak client is never running" )
        try
        {
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            {
                CFakeHostView view;
                view.AddFile( "/home/test/.local/share/Steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdC, "carol", "Carol", true, 5 ) ) );
                view.AddDir( "/home/test/.steam/steam" );      // there, but not a client
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && "/home/test/.local/share/Steam" == info.installPath && IdC == info.user.steamId );
            }
            {
                CFakeHostView view;
                view.AddFile( "/home/test/.var/app/com.valvesoftware.Steam/.local/share/Steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdA, "dave", "Dave", true, 5 ) ) );
                view.AddFile( "/home/test/.var/app/com.valvesoftware.Steam/.steam/steam.pid", "4242" );
                view.processes[ "4242" ] = "steam";
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.running && "Dave" == info.user.personaName );
            }
            {
                // two clients: the one that has the file with the users is the one in use
                CFakeHostView view;
                view.AddDir( "/home/test/.steam/steam/steamapps" );
                view.AddFile( "/home/test/.local/share/Steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdA, "erin", "Erin", true, 5 ) ) );
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && "/home/test/.local/share/Steam" == info.installPath && "Erin" == info.user.personaName );
            }
            {
                // the folder is a client and has no file with users: installed, nobody to name
                CFakeHostView view;
                view.AddDir( "/home/test/.steam/steam/steamapps" );
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.hasUser );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A broken, empty or huge loginusers.vdf still gives an installed client" )
        try
        {
            CFakeHostView view;
            AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            const CORE::CString path = "/home/test/.steam/steam/config/loginusers.vdf";

            view.AddFile( path, "" );
            ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.hasUser );
            view.AddFile( path, "\x01\x02 not a file {{{ \"" );
            ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.hasUser );
            view.AddFile( path, "\"users\" { }" );
            ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.hasUser );

            // cut short after the first user: that user is what there is
            view.AddFile( path, "\"users\" { \"" + CORE::UInt64ToString( IdA ) + "\" { \"AccountName\" \"x\" \"PersonaName\" \"Cut\" \"MostRecent\" \"1\" }" );
            ASSERT_TRUE( detector.Detect( info ) && info.hasUser && "Cut" == info.user.personaName );

            // larger than the limit: not read at all
            CORE::CString huge = Repeated( ' ', AUTHPLUGIN::STEAM::CSteamLocalDetector::MaxFileBytes + 1 );
            view.AddFile( path, huge + MakeLoginUsers( MakeUserBlock( IdA, "a", "Huge", true, 1 ) ) );
            ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.hasUser );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Windows: the folder of the registry, the fallbacks, and the process of the registry" )
        try
        {
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            {
                CFakeHostView view( AUTHPLUGIN::STEAM::STEAMHOST_WINDOWS, "C:/Users/test" );
                view.registryStrings[ "Software\\Valve\\Steam|SteamPath" ] = "d:/games/steam";
                view.AddFile( "d:/games/steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdA, "alice", "Alice P", false, 5 ) + MakeUserBlock( IdB, "bob", "Bob Q", true, 4 ) ) );
                view.AddFile( "d:/games/steam/steam.exe", "x" );
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && "d:/games/steam" == info.installPath && !info.running && IdB == info.user.steamId );

                // running by the process of the registry, and the user that is active
                view.registryDwords[ "Software\\Valve\\Steam\\ActiveProcess|pid" ] = "777";
                view.registryDwords[ "Software\\Valve\\Steam\\ActiveProcess|ActiveUser" ] = "12345678";
                ASSERT_TRUE( detector.Detect( info ) && !info.running );
                view.processes[ "777" ] = "Steam.exe";
                ASSERT_TRUE( detector.Detect( info ) && info.running && IdA == info.user.steamId );

                // the other value for the process id
                view.registryDwords.erase( "Software\\Valve\\Steam\\ActiveProcess|pid" );
                ASSERT_TRUE( detector.Detect( info ) && !info.running );
                view.registryDwords[ "Software\\Valve\\Steam|SteamPID" ] = "777";
                ASSERT_TRUE( detector.Detect( info ) && info.running );
            }
            {
                // no registry: the program files folder
                CFakeHostView view( AUTHPLUGIN::STEAM::STEAMHOST_WINDOWS, "C:/Users/test" );
                view.env[ "ProgramFiles(x86)" ] = "C:/Program Files (x86)";
                view.AddFile( "C:/Program Files (x86)/Steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdC, "carol", "Carol", true, 5 ) ) );
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && "C:/Program Files (x86)/Steam" == info.installPath && "Carol" == info.user.personaName );
                CORE::CString::StringVector folders;
                detector.GetCandidateInstallFolders( folders );
                ASSERT_TRUE( 1 == folders.size() );
            }
            {
                // a registry path that is not a client falls through to the next place
                CFakeHostView view( AUTHPLUGIN::STEAM::STEAMHOST_WINDOWS, "C:/Users/test" );
                view.registryStrings[ "Software\\Valve\\Steam|SteamPath" ] = "z:/gone";
                view.env[ "ProgramFiles" ] = "C:/Program Files";
                view.AddDir( "C:/Program Files/Steam/steamapps" );
                AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
                ASSERT_TRUE( detector.Detect( info ) && "C:/Program Files/Steam" == info.installPath );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: macOS: the application support folder and the process of its registry.vdf" )
        try
        {
            CFakeHostView view( AUTHPLUGIN::STEAM::STEAMHOST_MACOS, "/Users/test" );
            view.AddFile( "/Users/test/Library/Application Support/Steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdB, "bob", "Bob Q", true, 4 ) ) );
            view.AddFile( "/Users/test/Library/Application Support/Steam/registry.vdf", MakeRegistryVdf( 31337, 87654321 ) );
            AUTHPLUGIN::STEAM::CSteamLocalDetector detector( &view );
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            ASSERT_TRUE( detector.Detect( info ) && info.installed && !info.running && "Bob Q" == info.user.personaName );
            view.processes[ "31337" ] = "steam_osx";
            ASSERT_TRUE( detector.Detect( info ) && info.running && "Bob Q" == info.user.personaName );
            // the pid file of Linux is not looked for there
            CFakeHostView linuxFiles( AUTHPLUGIN::STEAM::STEAMHOST_MACOS, "/Users/test" );
            linuxFiles.AddFile( "/Users/test/Library/Application Support/Steam/config/loginusers.vdf", MakeLoginUsers( MakeUserBlock( IdB, "bob", "Bob Q", true, 4 ) ) );
            linuxFiles.AddFile( "/Users/test/.steam/steam.pid", "31337" );
            linuxFiles.processes[ "31337" ] = "steam";
            AUTHPLUGIN::STEAM::CSteamLocalDetector other( &linuxFiles );
            ASSERT_TRUE( other.Detect( info ) && !info.running );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUCEF_TESTFW_TESTCASE( "Test: The view of the machine reads a made up home folder from disk and changes nothing in it" )
        try
        {
            const CORE::CString home = MakeTestDirectory( "steam_home" );
            const CORE::CString steam = CORE::CombinePath( home, ".steam/steam" );
            const CORE::CString config = CORE::CombinePath( steam, "config" );
            ASSERT_TRUE( CORE::CreateDirs( config ) );
            const CORE::CString usersFile = CORE::CombinePath( config, "loginusers.vdf" );
            const CORE::CString pidFile = CORE::CombinePath( home, ".steam/steam.pid" );
            const CORE::CString users = MakeLoginUsers( MakeUserBlock( IdA, "alice", "Alice P", true, 1700000000ULL ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( usersFile, users ) );
            // the process of this test is not a Steam process: the file says it is the client's, the process table says it is not
            ASSERT_TRUE( CORE::WriteStringAsTextFile( pidFile, CORE::UInt32ToString( (UInt32) getpid() ) ) );

            const CORE::CString originalHome = CORE::GetEnv( "HOME" );
            CORE::SetEnvOverride( "HOME", home );

            AUTHPLUGIN::STEAM::CSteamLocalDetector detector;
            AUTHPLUGIN::STEAM::CSteamLocalInfo info;
            const bool detected = detector.Detect( info );

            CORE::SetEnvOverride( "HOME", originalHome );

            ASSERT_TRUE( detected && info.installed && !info.running && CORE::CString( steam ) == info.installPath );
            ASSERT_TRUE( info.hasUser && IdA == info.user.steamId && "Alice P" == info.user.personaName );

            // nothing was written or removed
            CORE::CString after;
            ASSERT_TRUE( CORE::LoadTextFileAsString( usersFile, after, false ) && after == users );
            ASSERT_TRUE( HasName( ListDirectory( config ), "loginusers.vdf" ) && 1 == ListDirectory( config ).size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The view of the machine finds a process by its id and its name" )
        try
        {
            const AUTHPLUGIN::STEAM::CSystemSteamHostView& view = AUTHPLUGIN::STEAM::CSystemSteamHostView::Instance();
            const UInt32 self = (UInt32) getpid();
            ASSERT_TRUE( view.IsProcessRunning( self, "" ) );
            ASSERT_TRUE( view.IsProcessRunning( self, "gucefAUTH" ) );
            ASSERT_TRUE( view.IsProcessRunning( self, "GUCEFAUTH" ) );
            ASSERT_FALSE( view.IsProcessRunning( self, "no-such-name-zz" ) );
            ASSERT_FALSE( view.IsProcessRunning( 0, "" ) );
            ASSERT_FALSE( view.IsProcessRunning( 4000000000U, "" ) );

            CORE::CString content;
            ASSERT_FALSE( view.ReadTextFile( "/definitely/not/here.vdf", 1000, content ) );
            ASSERT_FALSE( view.DirectoryExists( "" ) || view.FileExists( "" ) );
            CORE::CString value;
            UInt32 number = 1;
            ASSERT_FALSE( view.ReadRegistryString( "Software\\Valve\\Steam", "SteamPath", value ) || view.ReadRegistryDword( "Software\\Valve\\Steam", "pid", number ) );
            ASSERT_TRUE( AUTHPLUGIN::STEAM::STEAMHOST_LINUX == view.GetPlatform() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The view of the machine does not read a file over the size limit" )
        try
        {
            const CORE::CString dir = MakeTestDirectory( "steam_limit" );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            const CORE::CString file = CORE::CombinePath( dir, "big.vdf" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( file, Repeated( 'x', 5000 ) ) );
            CORE::CString content;
            const AUTHPLUGIN::STEAM::CSystemSteamHostView& view = AUTHPLUGIN::STEAM::CSystemSteamHostView::Instance();
            ASSERT_FALSE( view.ReadTextFile( file, 4999, content ) );
            ASSERT_TRUE( content.IsNULLOrEmpty() );
            ASSERT_TRUE( view.ReadTextFile( file, 5000, content ) && 5000 == strlen( content.C_String() ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif
}

/*-------------------------------------------------------------------------*/
