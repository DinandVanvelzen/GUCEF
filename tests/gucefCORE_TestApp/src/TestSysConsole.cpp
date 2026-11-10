/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLE_H
#include "CSysConsole.h"
#define GUCEF_CORE_CSYSCONSOLE_H
#endif /* GUCEF_CORE_CSYSCONSOLE_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLECLIENT_H
#include "CSysConsoleClient.h"
#define GUCEF_CORE_CSYSCONSOLECLIENT_H
#endif /* GUCEF_CORE_CSYSCONSOLECLIENT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSysConsole.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

typedef CORE::CString::StringVector TStrings;

/** Records the commands it gets and answers with the path, the name and the values */
class CRecordingHandler : public CORE::CISysConsoleCmdHandler
{
    public:

    UInt32 calls;
    CORE::CString lastPath;
    CORE::CString lastCommand;
    TStrings lastArgs;
    bool registerOnCall;

    CRecordingHandler( void ) : calls( 0 ), lastPath(), lastCommand(), lastArgs(), registerOnCall( false ) {}

    virtual bool OnSysConsoleCommand( const CORE::CString& path    ,
                                      const CORE::CString& command ,
                                      const TStrings& args         ,
                                      TStrings& resultdata         ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++calls;
        lastPath = path;
        lastCommand = command;
        lastArgs = args;
        resultdata.push_back( "ran " + command );

        // a command may use the console while it runs
        if ( registerOnCall )
            CORE::CCoreGlobal::Instance()->GetSysConsole().RegisterCmd( "Test\\SysConsole\\Late", "LateCmd", TStrings(), this );
        return true;
    }
};

/** A console client with its protected interface opened up for the tests */
class CTestClient : public CORE::CSysConsoleClient
{
    public:

    using CORE::CSysConsoleClient::ProcessUserInput;
    using CORE::CSysConsoleClient::JumpTo;
    using CORE::CSysConsoleClient::GetPath;
    using CORE::CSysConsoleClient::GetDirList;
    using CORE::CSysConsoleClient::GetCmdList;
    using CORE::CSysConsoleClient::GetCompletions;
};

bool
Contains( const TStrings& list, const CORE::CString& value )
{
    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ] == value )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSysConsoleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SysConsole" );

    CORE::CSysConsole& console = CORE::CCoreGlobal::Instance()->GetSysConsole();

    GUCEF_TESTFW_TESTCASE( "SysConsole: A command runs by its name from anywhere, by its path, and from its dir" )
    try
    {
        CRecordingHandler handler;
        TStrings args;
        args.push_back( "x" );
        args.push_back( "y" );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Math", "Add", args, &handler ) );
        ASSERT_TRUE( console.HasCmd( "Test/SysConsole/Math", "Add" ) );

        CTestClient client;
        TStrings result;
        ASSERT_TRUE( client.ProcessUserInput( "Add 1 2", result ) );
        ASSERT_TRUE( 1 == handler.calls && "Add" == handler.lastCommand && "Test\\SysConsole\\Math" == handler.lastPath );
        ASSERT_TRUE( 2 == handler.lastArgs.size() && "1" == handler.lastArgs[ 0 ] && "2" == handler.lastArgs[ 1 ] );
        ASSERT_TRUE( 1 == result.size() && "ran Add" == result[ 0 ] );

        result.clear();
        ASSERT_TRUE( client.ProcessUserInput( "\\Test\\SysConsole\\Math\\Add 3 4", result ) );
        ASSERT_TRUE( 2 == handler.calls );

        // moving through the dirs
        result.clear();
        ASSERT_TRUE( client.ProcessUserInput( "cd Test/SysConsole", result ) );
        ASSERT_TRUE( "Test\\SysConsole" == client.GetPath() );
        ASSERT_TRUE( Contains( client.GetDirList(), "Math" ) );
        ASSERT_TRUE( client.ProcessUserInput( "Math\\Add 5 6", result ) );
        ASSERT_TRUE( 3 == handler.calls );
        ASSERT_TRUE( client.ProcessUserInput( "cd Math", result ) );
        ASSERT_TRUE( Contains( client.GetCmdList(), "Add" ) );
        ASSERT_TRUE( client.ProcessUserInput( "cd ..", result ) );
        ASSERT_TRUE( "Test\\SysConsole" == client.GetPath() );
        ASSERT_FALSE( client.ProcessUserInput( "cd NoSuchDir", result ) );
        ASSERT_TRUE( "Test\\SysConsole" == client.GetPath() );
        result.clear();
        ASSERT_TRUE( client.ProcessUserInput( "pwd", result ) );
        ASSERT_TRUE( 1 == result.size() && "\\Test\\SysConsole" == result[ 0 ] );

        console.UnregisterCmdsOfHandler( &handler );
        ASSERT_FALSE( console.HasCmd( "Test\\SysConsole\\Math", "Add" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SysConsole: Unknown and ambiguous commands and wrong value counts do not run" )
    try
    {
        CRecordingHandler handler;
        TStrings args;
        args.push_back( "value" );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\A", "Same", args, &handler ) );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\B", "Same", args, &handler ) );
        TStrings rest;
        rest.push_back( "first" );
        rest.push_back( "others..." );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\A", "Echo", rest, &handler ) );

        CTestClient client;
        TStrings result;
        ASSERT_FALSE( client.ProcessUserInput( "NoSuchCommand", result ) );
        ASSERT_TRUE( 0 == handler.calls && !result.empty() );

        // the same name in two dirs: the paths are listed, nothing runs
        result.clear();
        ASSERT_FALSE( client.ProcessUserInput( "Same 1", result ) );
        ASSERT_TRUE( 0 == handler.calls && 3 == result.size() );
        ASSERT_TRUE( client.ProcessUserInput( "Test\\SysConsole\\B\\Same 1", result ) );
        ASSERT_TRUE( 1 == handler.calls && "Test\\SysConsole\\B" == handler.lastPath );

        // exactly as many values as the command has
        ASSERT_FALSE( client.ProcessUserInput( "\\Test\\SysConsole\\A\\Same", result ) );
        ASSERT_FALSE( client.ProcessUserInput( "\\Test\\SysConsole\\A\\Same 1 2", result ) );
        ASSERT_TRUE( 1 == handler.calls );

        // a last value with "..." takes the rest, also none
        ASSERT_TRUE( client.ProcessUserInput( "Echo a b c", result ) );
        ASSERT_TRUE( 3 == handler.lastArgs.size() );
        ASSERT_TRUE( client.ProcessUserInput( "Echo a", result ) );
        ASSERT_FALSE( client.ProcessUserInput( "Echo", result ) );
        ASSERT_TRUE( 3 == handler.calls );

        console.UnregisterCmdsOfHandler( &handler );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SysConsole: Words in quotes keep their spaces, a quote that is not closed is an error" )
    try
    {
        TStrings words;
        ASSERT_TRUE( CORE::CSysConsoleClient::SplitCommandLine( "  say  \"hello world\"  x\"y z\" ", words ) );
        ASSERT_TRUE( 3 == words.size() && "say" == words[ 0 ] && "hello world" == words[ 1 ] && "xy z" == words[ 2 ] );
        ASSERT_TRUE( CORE::CSysConsoleClient::SplitCommandLine( "path \\a\\b \"q\\\"uote\\\\\"", words ) );
        ASSERT_TRUE( 3 == words.size() && "\\a\\b" == words[ 1 ] && "q\"uote\\" == words[ 2 ] );
        ASSERT_TRUE( CORE::CSysConsoleClient::SplitCommandLine( "\"\"", words ) );
        ASSERT_TRUE( 1 == words.size() && words[ 0 ].IsNULLOrEmpty() );
        ASSERT_FALSE( CORE::CSysConsoleClient::SplitCommandLine( "say \"open", words ) );

        // a command named like a console command is still a command
        CRecordingHandler handler;
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole", "cdrom", TStrings(), &handler ) );
        CTestClient client;
        TStrings result;
        ASSERT_TRUE( client.ProcessUserInput( "cdrom", result ) );
        ASSERT_TRUE( 1 == handler.calls );
        console.UnregisterCmdsOfHandler( &handler );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SysConsole: Aliases run their command, also registered through the console itself" )
    try
    {
        CRecordingHandler handler;
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Deep\\Dir", "Hidden", TStrings(), &handler ) );
        ASSERT_TRUE( console.RegisterAlias( "hidden", "Test\\SysConsole\\Deep\\Dir", "Hidden" ) );
        ASSERT_FALSE( console.RegisterAlias( "hidden", "Test\\SysConsole", "Other" ) );

        CTestClient client;
        TStrings result;
        ASSERT_TRUE( client.ProcessUserInput( "hidden", result ) );
        ASSERT_TRUE( 1 == handler.calls );

        ASSERT_TRUE( client.ProcessUserInput( "RegisterAlias h2 Test/SysConsole/Deep/Dir Hidden", result ) );
        ASSERT_TRUE( client.ProcessUserInput( "h2", result ) );
        ASSERT_TRUE( 2 == handler.calls );
        ASSERT_TRUE( client.ProcessUserInput( "UnregisterAlias h2", result ) );
        ASSERT_FALSE( client.ProcessUserInput( "h2", result ) );
        ASSERT_TRUE( console.UnregisterAlias( "hidden", CORE::CString::Empty, CORE::CString::Empty ) );
        ASSERT_FALSE( client.ProcessUserInput( "hidden", result ) );
        console.UnregisterCmdsOfHandler( &handler );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SysConsole: Completion offers aliases, commands and dirs that start with what was typed" )
    try
    {
        CRecordingHandler handler;
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Video", "vid_restart", TStrings(), &handler ) );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Video", "vid_mode", TStrings(), &handler ) );
        ASSERT_TRUE( console.RegisterAlias( "vidalias", "Test\\SysConsole\\Video", "vid_mode" ) );

        CTestClient client;
        TStrings matches = client.GetCompletions( "vid" );
        ASSERT_TRUE( 3 == matches.size() && Contains( matches, "vid_restart" ) && Contains( matches, "vid_mode" ) && Contains( matches, "vidalias" ) );
        ASSERT_TRUE( 1 == client.GetCompletions( "vid_r" ).size() );
        ASSERT_TRUE( client.JumpTo( "\\Test\\SysConsole" ) );
        ASSERT_TRUE( Contains( client.GetCompletions( "Vi" ), "Video\\" ) );

        console.UnregisterAlias( "vidalias", CORE::CString::Empty, CORE::CString::Empty );
        console.UnregisterCmdsOfHandler( &handler );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SysConsole: Unregistering takes commands away and empty dirs with them" )
    try
    {
        CRecordingHandler handler;
        CRecordingHandler other;
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Gone\\Sub", "One", TStrings(), &handler ) );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Gone", "Two", TStrings(), &handler ) );
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole\\Stays", "Three", TStrings(), &other ) );

        console.UnregisterCmd( "Test\\SysConsole\\Gone\\Sub", "One" );
        ASSERT_FALSE( console.HasCmd( "Test\\SysConsole\\Gone\\Sub", "One" ) );
        CTestClient client;
        ASSERT_FALSE( client.JumpTo( "\\Test\\SysConsole\\Gone\\Sub" ) );
        ASSERT_TRUE( client.JumpTo( "\\Test\\SysConsole\\Gone" ) );

        console.UnregisterCmdsOfHandler( &handler );
        ASSERT_FALSE( client.JumpTo( "\\Test\\SysConsole\\Gone" ) );
        ASSERT_TRUE( console.HasCmd( "Test\\SysConsole\\Stays", "Three" ) );
        console.UnregisterCmdsOfHandler( &other );

        // names with spaces or separators, and commands without a handler, are refused
        ASSERT_FALSE( console.RegisterCmd( "Test", "two words", TStrings(), &handler ) );
        ASSERT_FALSE( console.RegisterCmd( "Test", "a\\b", TStrings(), &handler ) );
        ASSERT_FALSE( console.RegisterCmd( "Test", "NoHandler", TStrings(), GUCEF_NULL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SysConsole: A command can register commands while it runs" )
    try
    {
        CRecordingHandler handler;
        handler.registerOnCall = true;
        ASSERT_TRUE( console.RegisterCmd( "Test\\SysConsole", "Registers", TStrings(), &handler ) );
        CTestClient client;
        TStrings result;
        ASSERT_TRUE( client.ProcessUserInput( "Registers", result ) );
        ASSERT_TRUE( console.HasCmd( "Test\\SysConsole\\Late", "LateCmd" ) );
        handler.registerOnCall = false;
        ASSERT_TRUE( client.ProcessUserInput( "LateCmd", result ) );
        ASSERT_TRUE( 2 == handler.calls );
        console.UnregisterCmdsOfHandler( &handler );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
