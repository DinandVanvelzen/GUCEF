/*
 *  gucefCOM_TestApp: GUCEF test application
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

#include <sys/stat.h>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COM_CSERVICEADMINENDPOINT_H
#include "gucefCOM_CServiceAdminEndpoint.h"
#define GUCEF_COM_CSERVICEADMINENDPOINT_H
#endif /* GUCEF_COM_CSERVICEADMINENDPOINT_H ? */

#ifndef GUCEF_COM_CSERVICEADMINCLIENT_H
#include "gucefCOM_CServiceAdminClient.h"
#define GUCEF_COM_CSERVICEADMINCLIENT_H
#endif /* GUCEF_COM_CSERVICEADMINCLIENT_H ? */

#ifndef GUCEF_COM_CSERVICEADMINCONTRACTS_H
#include "gucefCOM_CServiceAdminContracts.h"
#define GUCEF_COM_CSERVICEADMINCONTRACTS_H
#endif /* GUCEF_COM_CSERVICEADMINCONTRACTS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestServiceAdmin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE( t )   { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "ServiceAdmin test: requirement failed: " #t ); return; } }

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Answers by command: status OK, bad INVALID_ARGUMENT, later NOT_IMPLEMENTED, fail FAILED; records what it got */
class CTestAdminHandler : public COM::CIServiceAdminHandler
{
    public:

    MT::CMutex lock;
    UInt32 calls;
    CORE::CString lastCommand;
    TArgumentMap lastArguments;

    CTestAdminHandler( void ) : COM::CIServiceAdminHandler(), lock(), calls( 0 ), lastCommand(), lastArguments() {}

    virtual COM::TServiceAdminStatus HandleAdminCommand( const CORE::CString& command  ,
                                                         const TArgumentMap& arguments ,
                                                         CORE::CDataNode& result       ,
                                                         CORE::CString& message        ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex scope( lock );
            ++calls;
            lastCommand = command;
            lastArguments = arguments;
        }
        if ( "status" == command )
        {
            result.SetName( "status" );
            result.SetAttribute( "state", "READY" );
            TArgumentMap::const_iterator i = arguments.find( "echo" );
            if ( i != arguments.end() )
                result.SetAttribute( "echo", (*i).second );
            return COM::SERVICEADMIN_OK;
        }
        if ( "bad" == command )
        {
            message = "the argument kind is missing";
            return COM::SERVICEADMIN_INVALID_ARGUMENT;
        }
        if ( "later" == command )
        {
            message = "not built yet";
            return COM::SERVICEADMIN_NOT_IMPLEMENTED;
        }
        message = "it did not work";
        return COM::SERVICEADMIN_FAILED;
    }
};

/** Drives a pulse generator on a thread of its own: the service side, while the test thread runs the client */
class CTestPulseThread : public MT::CActiveObject
{
    public:

    CORE::PulseGeneratorPtr pulse;

    explicit CTestPulseThread( const CORE::PulseGeneratorPtr& pulseGenerator ) : MT::CActiveObject(), pulse( pulseGenerator ) {}
    virtual ~CTestPulseThread() { Deactivate( false, true ); }

    virtual bool OnThreadStart( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        return true;
    }

    virtual bool OnThreadCycle( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        pulse->ForceDirectPulse();
        return false;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

CORE::CString
AdminTestDirectory( const char* name )
{GUCEF_TRACE;

    CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( path, "ServiceAdminTest" );
    CORE::AppendToPath( path, name );
    if ( CORE::DirExists( path ) )
        CORE::DeleteDir( path, true );
    CORE::CreateDirs( path );
    return path;
}

/** The service side of a test: an endpoint on a pulse generator of its own, driven by a thread while calls run */
class CAdminServiceSide
{
    public:

    CTestAdminHandler handler;
    CORE::PulseGeneratorPtr pulse;
    COM::CServiceAdminEndpoint endpoint;
    CTestPulseThread* thread;

    CAdminServiceSide( void )
        : handler(), pulse( CORE::CPulseGenerator::CreateSharedObj() ), endpoint( handler, pulse ), thread( GUCEF_NULL )
    {
        pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    }

    ~CAdminServiceSide()
    {
        StopThread();
        endpoint.Close();
        for ( UInt32 i=0; i<20; ++i )
        {
            pulse->ForceDirectPulse();
            MT::PrecisionDelay( 1 );
        }
    }

    void StartThread( void )
    {
        thread = GUCEF_NEW CTestPulseThread( pulse );
        thread->Activate( GUCEF_NULL, 1, 1 );
    }

    void StopThread( void )
    {
        if ( GUCEF_NULL != thread )
        {
            thread->Deactivate( false, true );
            GUCEF_DELETE thread;
            thread = GUCEF_NULL;
            pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        }
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestAdminTokenAndCalls( void )
{GUCEF_TRACE;

    const CORE::CString stateDir = AdminTestDirectory( "calls" );
    CAdminServiceSide service;
    CORE::CString error;
    REQUIRE_TRUE( service.endpoint.Open( stateDir, 0, error ) );
    ASSERT_TRUE( service.endpoint.IsOpen() );
    ASSERT_TRUE( 0 != service.endpoint.GetBoundPort() );

    // the token file: 64 hexadecimal characters, owner only
    const CORE::CString tokenPath = CORE::CombinePath( stateDir, COM::CServiceAdminEndpoint::TokenFileName );
    ASSERT_TRUE( CORE::FileExists( tokenPath ) );
    CORE::CString tokenHex;
    ASSERT_TRUE( COM::CServiceAdminEndpoint::ReadTokenFile( stateDir, tokenHex, error ) );
    ASSERT_TRUE( 64 == tokenHex.Length() );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    struct stat info;
    ASSERT_TRUE( 0 == stat( tokenPath.C_String(), &info ) );
    ASSERT_TRUE( 0600 == ( info.st_mode & 0777 ) );
    #endif

    service.StartThread();
    const UInt16 port = service.endpoint.GetBoundPort();

    // the right token: the handler gets the command and its arguments, the result comes back
    COM::CServiceAdminClient::TArgumentMap arguments;
    arguments[ "echo" ] = "hello";
    CORE::CDataNode result;
    COMCORE::CRpcStatus status;
    ASSERT_TRUE( COM::CServiceAdminClient::RunWithTokenFile( port, stateDir, "status", arguments, result, status, 10000 ) );
    ASSERT_TRUE( status.IsOk() );
    ASSERT_TRUE( "READY" == result.GetAttributeValueOrChildValueByName( "state" ).AsString() );
    ASSERT_TRUE( "hello" == result.GetAttributeValueOrChildValueByName( "echo" ).AsString() );
    {
        MT::CScopeMutex scope( service.handler.lock );
        ASSERT_TRUE( 1 == service.handler.calls );
        ASSERT_TRUE( "status" == service.handler.lastCommand );
    }

    // the handler's answers as RPC status codes
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, tokenHex, "bad", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == status.code );
    ASSERT_TRUE( "the argument kind is missing" == status.message );
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, tokenHex, "later", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( COMCORE::RPC_STATUS_UNIMPLEMENTED == status.code );
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, tokenHex, "fail", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == status.code );

    // a wrong token, and a token of another length: refused before the handler, counted
    CORE::CString wrong = tokenHex;
    wrong = ( '0' == wrong[ 0 ] ? CORE::CString( "1" ) : CORE::CString( "0" ) ) + wrong.CutChars( 1, true );
    const UInt32 callsBefore = service.handler.calls;
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, wrong, "status", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( COMCORE::RPC_STATUS_UNAUTHENTICATED == status.code );
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, "00ff", "status", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( COMCORE::RPC_STATUS_UNAUTHENTICATED == status.code );
    ASSERT_TRUE( 2 == service.endpoint.GetRefusedCount() );
    ASSERT_TRUE( callsBefore == service.handler.calls );

    // not hexadecimal: refused by the client itself
    ASSERT_FALSE( COM::CServiceAdminClient::Run( port, "not hex!", "status", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );

    service.StopThread();
    service.endpoint.Close();
    ASSERT_FALSE( service.endpoint.IsOpen() );
    ASSERT_FALSE( CORE::FileExists( tokenPath ) );

    // nobody listens any more: the client says so within its time
    ASSERT_FALSE( COM::CServiceAdminClient::Run( port, tokenHex, "status", COM::CServiceAdminClient::TArgumentMap(), result, status, 3000 ) );
    ASSERT_FALSE( status.IsOk() );
}

/*-------------------------------------------------------------------------*/

static void
TestAdminCommandFilterAndTakenPort( void )
{GUCEF_TRACE;

    const CORE::CString stateDir = AdminTestDirectory( "filter" );
    CAdminServiceSide service;
    CORE::CString::StringSet commands;
    commands.insert( "status" );
    service.endpoint.SetCommands( commands );
    CORE::CString error;
    REQUIRE_TRUE( service.endpoint.Open( stateDir, 0, error ) );
    const UInt16 port = service.endpoint.GetBoundPort();
    CORE::CString tokenHex;
    REQUIRE_TRUE( COM::CServiceAdminEndpoint::ReadTokenFile( stateDir, tokenHex, error ) );

    // a second endpoint on the same port fails, and the first one's token file stays as it is
    CAdminServiceSide second;
    const CORE::CString secondDir = AdminTestDirectory( "filter2" );
    ASSERT_FALSE( second.endpoint.Open( secondDir, port, error ) );
    ASSERT_FALSE( error.IsNULLOrEmpty() );
    CORE::CString tokenAfter;
    ASSERT_TRUE( COM::CServiceAdminEndpoint::ReadTokenFile( stateDir, tokenAfter, error ) );
    ASSERT_TRUE( tokenHex == tokenAfter );
    ASSERT_FALSE( second.endpoint.Open( stateDir, port, error ) );
    ASSERT_TRUE( COM::CServiceAdminEndpoint::ReadTokenFile( stateDir, tokenAfter, error ) );
    ASSERT_TRUE( tokenHex == tokenAfter );

    service.StartThread();
    CORE::CDataNode result;
    COMCORE::CRpcStatus status;
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, tokenHex, "pair-open", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( COMCORE::RPC_STATUS_NOT_FOUND == status.code );
    ASSERT_TRUE( 0 == service.handler.calls );
    ASSERT_TRUE( COM::CServiceAdminClient::Run( port, tokenHex, "status", COM::CServiceAdminClient::TArgumentMap(), result, status, 10000 ) );
    ASSERT_TRUE( status.IsOk() );
    service.StopThread();

    // a missing token file is reported, not guessed
    CORE::CString missing;
    ASSERT_FALSE( COM::CServiceAdminEndpoint::ReadTokenFile( secondDir, missing, error ) );
}

/*-------------------------------------------------------------------------*/

static void
TestAdminCommandLine( void )
{GUCEF_TRACE;

    CORE::CString command;
    COM::CServiceAdminClient::TArgumentMap arguments;
    CORE::CString error;

    CORE::CString::StringVector words;
    words.push_back( "pair-open" );
    words.push_back( "--kind" );
    words.push_back( "viewer" );
    words.push_back( "--roles" );
    words.push_back( "viewer,curator" );
    words.push_back( "--confirm" );
    ASSERT_TRUE( COM::CServiceAdminClient::ParseCommandLine( words, command, arguments, error ) );
    ASSERT_TRUE( "pair-open" == command );
    ASSERT_TRUE( 3 == arguments.size() );
    ASSERT_TRUE( "viewer" == arguments[ "kind" ] );
    ASSERT_TRUE( "viewer,curator" == arguments[ "roles" ] );
    ASSERT_TRUE( "true" == arguments[ "confirm" ] );

    words.clear();
    words.push_back( "status" );
    ASSERT_TRUE( COM::CServiceAdminClient::ParseCommandLine( words, command, arguments, error ) );
    ASSERT_TRUE( "status" == command && arguments.empty() );

    words.clear();
    ASSERT_FALSE( COM::CServiceAdminClient::ParseCommandLine( words, command, arguments, error ) );
    words.push_back( "--kind" );
    ASSERT_FALSE( COM::CServiceAdminClient::ParseCommandLine( words, command, arguments, error ) );
    words.clear();
    words.push_back( "revoke" );
    words.push_back( "d-123" );
    ASSERT_FALSE( COM::CServiceAdminClient::ParseCommandLine( words, command, arguments, error ) );
    ASSERT_FALSE( error.IsNULLOrEmpty() );
    words.clear();
    words.push_back( "revoke" );
    words.push_back( "--" );
    ASSERT_FALSE( COM::CServiceAdminClient::ParseCommandLine( words, command, arguments, error ) );
}

/*-------------------------------------------------------------------------*/

void
PerformServiceAdminTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SERVICE ADMIN ENDPOINT TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceAdmin" );

    GUCEF_TESTFW_TESTCASE( "Test: contract gucef.service.admin 1.0 builds, its token field and method are sensitive" )
        try
        {
            COMCORE::CRpcContractPtr contract = COM::CServiceAdminContracts::GetContract();
            ASSERT_TRUE( !contract.IsNULL() );
            ASSERT_TRUE( COM::CServiceAdminContracts::GetContract() == contract );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the admin endpoint on loopback: owner-only token file, calls with the token reach the handler, its answers map to status codes, refusals without it, Close removes the file" )
        try
        {
            TestAdminTokenAndCalls();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: unknown commands answer NOT_FOUND with SetCommands; a second endpoint on a taken port fails and leaves the token file of the first" )
        try
        {
            TestAdminCommandFilterAndTakenPort();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the admin command line: a command, named values, flags and refused forms" )
        try
        {
            TestAdminCommandLine();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
