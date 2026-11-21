/*
 *  gucefLabLink_TestApp: tests of gucefLabLink
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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#if defined( __linux__ )
  #include <dirent.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "gucef_test_tls_server.h"

#include "gucefLabLink_Cluster.h"
#include "gucefLabLink_PasswordHash.h"
#include "gucefLabLink_RegistryHubView.h"
#include "gucefLabLink_WebApi.h"
#include "gucefLabLink_WebSite.h"
#include "gucefLabLink_WebUsers.h"
#include "TestLabLinkWeb.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEFLabLink;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
LoadCryptography( void )
{
    COM::CComGlobal::Instance();
    DRN::CDRNGlobal::Instance();
    if ( CPasswordHash::IsArgon2idAvailable() )
        return true;
    // as a codec plugin, as gucefLabLink loads it: no trial of the other plugin kinds
    CORE::CPluginControl& plugins = CORE::CCoreGlobal::Instance()->GetPluginControl();
    CORE::CPluginMetaData metaData;
    metaData.SetFullModulePath( CORE::RelativePath( "$MODULEDIR$" ) );
    metaData.SetPluginType( "GucefStandardCodecPlugin" );
    metaData.SetModuleFilename( "codecspluginMONOCYPHER" );
    if ( !plugins.AddPluginMetaData( metaData, "test", true ) )
    {
        metaData.SetModuleFilename( "codecspluginMONOCYPHER_d" );
        plugins.AddPluginMetaData( metaData, "test", true );
    }
    return CPasswordHash::IsArgon2idAvailable() && DRN::CDRNCrypto::IsKeyAgreementAvailable();
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ReadText( const CORE::CString& path )
{
    CORE::CDynamicBuffer content;
    if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path ) )
        return CORE::CString::Empty;
    return ( 0 == content.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

bool
WriteText( const CORE::CString& path, const CORE::CString& text )
{
    FILE* file = fopen( path.C_String(), "wb" );
    if ( GUCEF_NULL == file )
        return false;
    fwrite( text.C_String(), 1, text.Length(), file );
    fclose( file );
    return true;
}

/*-------------------------------------------------------------------------*/

/** A cheap hash, so the tests do not spend seconds on it; the service uses the defaults */
SPasswordParams
CheapParams( void )
{
    SPasswordParams params;
    params.algorithm = CPasswordHash::Argon2id;
    params.memoryInKiB = 1024;
    params.iterations = 1;
    params.lanes = 1;
    return params;
}

/*-------------------------------------------------------------------------*/

/** Everything the log says while it is there */
class CLogCapture : public CORE::CILogger
{
    public:

    CLogCapture( void ) : CORE::CILogger(), m_text(), m_lock() { CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this ); }
    virtual ~CLogCapture() { CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this ); }

    CORE::CString GetText( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_text;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_text += logMessage;
        m_text += "\n";
    }
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    CORE::CString m_text;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** A request as the server hands it over */
WEB::CHttpRequestData
Request( const char* method, const char* path, const CORE::CString& body = CORE::CString::Empty, bool csrf = true )
{
    WEB::CHttpRequestData request;
    request.requestType = method;
    request.requestUri = path;
    request.requestHost = "127.0.0.1:47813";
    if ( !body.IsNULLOrEmpty() )
    {
        request.content.Append( body.C_String(), body.Length() );
        request.contentRepresentation = "application/x-www-form-urlencoded";
        request.otherHeaders[ "content-length" ] = CORE::ToString( body.Length() );
    }
    if ( csrf )
        request.otherHeaders[ "x-lablink-csrf" ] = "1";
    return request;
}

/*-------------------------------------------------------------------------*/

CORE::CString
BodyOf( const WEB::CHttpResponseData& response )
{
    const UInt32 size = response.content.GetDataSize();
    return size > 0 ? CORE::CString( static_cast< const char* >( response.content.GetConstBufferPtr() ), size ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
HeaderOf( const WEB::CHttpResponseData& response, const char* name )
{
    WEB::CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.find( name );
    return i == response.otherHeaders.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

/** The session id of a Set-Cookie value */
CORE::CString
SessionIdOf( const WEB::CHttpResponseData& response )
{
    const CORE::CString cookie = HeaderOf( response, "Set-Cookie" );
    const Int32 equals = cookie.HasChar( '=' );
    const Int32 semicolon = cookie.HasChar( ';' );
    if ( equals < 0 || semicolon < equals )
        return CORE::CString::Empty;
    return cookie.SubstrFromRange( (UInt32) equals + 1, (UInt32) semicolon );
}

/*-------------------------------------------------------------------------*/

/** Answers a request as the website does, with the password job, if any, run on this thread. @return the status */
int
Ask( CLabWebApi& api, const WEB::CHttpRequestData& request, const CORE::CString& peer, UInt64 nowInMs, WEB::CHttpResponseData& response,
     bool* hashed = GUCEF_NULL, SPasswordJob* jobSeen = GUCEF_NULL )
{
    response = WEB::CHttpResponseData();
    SPasswordJob job;
    const bool answered = api.Handle( request, &peer, nowInMs, response, job );
    if ( GUCEF_NULL != hashed )
        *hashed = !answered;
    if ( !answered )
    {
        if ( GUCEF_NULL != jobSeen )
            *jobSeen = job;
        CLabWebApi::RunPasswordJob( job );
        api.CompletePasswordJob( job, nowInMs, response );
    }
    return response.statusCode;
}

/*-------------------------------------------------------------------------*/

/** The same with the session cookie */
int
AskWith( CLabWebApi& api, WEB::CHttpRequestData request, const CORE::CString& sessionId, UInt64 nowInMs, WEB::CHttpResponseData& response )
{
    // where CHTTPServer puts the Cookie field
    request.transactionID = "other=1; " + api.GetCookieName() + "=" + sessionId;
    return Ask( api, request, "127.0.0.1", nowInMs, response );
}

/*-------------------------------------------------------------------------*/

CORE::CString
LoginBody( const CORE::CString& user, const CORE::CString& password, bool stay = false )
{
    return "username=" + user + "&password=" + password + ( stay ? "&stay=1" : "" );
}

/*-------------------------------------------------------------------------*/

/** A hub without the network: a registry, its view, users in a state directory */
class CWebFixture
{
    public:

    TESTFW::CScopedTestDir dir;
    CStore store;
    CWebUserStore users;
    CNodeRegistry registry;
    CRegistryHubView hub;
    CAddressRanges ranges;
    SWebSettings settings;

    explicit CWebFixture( const char* name )
        : dir( name )
        , store( CORE::CombinePath( dir.GetPath(), "state" ) )
        , users( store )
        , registry()
        , hub( registry, "testhub" )
        , ranges()
        , settings()
    {
        settings.passwordParams = CheapParams();
        CORE::CString problem;
        users.Load( problem );
    }

    bool AddUser( const char* name, const char* role, const char* password, bool disabled = false )
    {
        SWebUser user;
        user.name = name;
        user.role = role;
        user.disabled = disabled;
        user.createdInUnixSecs = 1700000000;
        CORE::CString problem;
        return CPasswordHash::MakeRecord( password, settings.passwordParams, user.record ) && users.Add( user, problem );
    }
};

/*-------------------------------------------------------------------------*/

/** Runs a program to its end; @return its exit code, -1 when it did not run */
Int32
Run( const CORE::CString& exe, const CORE::CProcess::TStringVector& args, const CORE::CProcess::TStringMap& environment, CORE::CString& output )
{
    std::string text;
    Int32 exitCode = -1;
    if ( !RunProgram( exe, args, environment, 120000, text, exitCode ) )
        return -1;
    output = text.c_str();
    return exitCode;
}

/*-------------------------------------------------------------------------*/

/** A command of gucefLabLink with the password on its standard input, as an administrator types it in */
Int32
RunWithPassword( const CORE::CString& program, const CORE::CString& stateDir, const CORE::CString& password, const CORE::CString& command,
                 CORE::CString& output )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "-c" );
    args.push_back( "printf '%s\\n' \"$LABLINK_TEST_PASSWORD\" | \"$LABLINK_PROGRAM\" --state-dir \"$LABLINK_STATE\" " + command );
    CORE::CProcess::TStringMap environment;
    environment[ "LABLINK_TEST_PASSWORD" ] = password;
    environment[ "LABLINK_PROGRAM" ] = program;
    environment[ "LABLINK_STATE" ] = stateDir;
    environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
    return Run( "/bin/sh", args, environment, output );
}

/*-------------------------------------------------------------------------*/

bool
IsCurlAvailable( void )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "--version" );
    CORE::CString output;
    return 0 == Run( "curl", args, CORE::CProcess::TStringMap(), output );
}

/*-------------------------------------------------------------------------*/

/** curl with the arguments and the status code written last; @return the status, 0 when there was no answer */
int
Curl( CORE::CProcess::TStringVector args, CORE::CString& body )
{
    args.insert( args.begin(), "-s" );
    args.push_back( "-w" );
    args.push_back( "\n%{http_code}" );
    CORE::CString output;
    Run( "curl", args, CORE::CProcess::TStringMap(), output );
    const Int32 lastBreak = output.HasChar( '\n', false );
    if ( lastBreak < 0 )
    {
        body = output;
        return 0;
    }
    body = output.SubstrToIndex( (UInt32) lastBreak, true );
    return (int) CORE::StringToUInt32( output.CutChars( (UInt32) lastBreak + 1, true ), 0 );
}

/*-------------------------------------------------------------------------*/

bool
WaitForPort( UInt16 port, UInt32 timeoutInMs )
{
    const UInt64 deadline = NowInMs() + timeoutInMs;
    while ( NowInMs() < deadline )
    {
        if ( TESTFW::CTlsTestServer::CanConnect( port ) )
            return true;
        MT::ThreadDelay( 50 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The hub as a program of its own: the cluster role, with the arguments given beyond the lab's */
class CHubProgram
{
    public:

    TESTFW::CScopedTestDir dir;
    CORE::CString program;
    UInt16 labPort;
    UInt16 accessPort;
    CORE::CProcess process;

    explicit CHubProgram( const char* name )
        : dir( name )
        , program( CORE::RelativePath( "$MODULEDIR$/gucefLabLink" ) )
        , labPort( TESTFW::CTlsTestServer::FindFreePort() )
        , accessPort( TESTFW::CTlsTestServer::FindFreePort() )
        , process()
    {
        while ( accessPort == labPort )
            accessPort = TESTFW::CTlsTestServer::FindFreePort();
        CORE::CreateDirs( Path( "mcp" ) );
        CStore store( Path( "state" ) );
        CORE::CString problem;
        store.WriteSecret( MakeLabSecret(), problem );
    }

    ~CHubProgram()
    {
        Stop();
    }

    CORE::CString Path( const char* name ) const { return CORE::CombinePath( dir.GetPath(), name ); }

    bool Start( const CORE::CProcess::TStringVector& extra )
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "--role" ); args.push_back( "cluster" );
        args.push_back( "--state-dir" ); args.push_back( Path( "state" ) );
        args.push_back( "--cluster-name" ); args.push_back( "testhub" );
        args.push_back( "--lab-port" ); args.push_back( CORE::ToString( labPort ) );
        args.push_back( "--access-port" ); args.push_back( CORE::ToString( accessPort ) );
        args.push_back( "--no-mdns" );
        args.push_back( "--no-broadcast" );
        args.push_back( "--k3s-bin" ); args.push_back( Path( "no-k3s" ) );
        args.push_back( "--mcp-dir" ); args.push_back( Path( "mcp" ) );
        args.push_back( "--db" ); args.push_back( "none" );
        args.push_back( "--allow" ); args.push_back( "127.0.0.0/8" );
        for ( size_t i=0; i<extra.size(); ++i )
            args.push_back( extra[ i ] );
        CORE::CProcess::TStringMap environment;
        environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
        process.SetIsolateFromParent( true );
        return process.Launch( program, args, CORE::CString::Empty, environment ) && WaitForPort( labPort, 30000 );
    }

    void Stop( void )
    {
        if ( process.IsRunning() )
            process.Terminate( true, 15000 );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLabLinkWebTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "LabLinkWeb" );

    GUCEF_TESTFW_TESTCASE( "Web: password records: a round trip, a wrong password, new salt every time, PBKDF2, the dummy record, an upgrade of the parameters" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            const SPasswordParams cheap = CheapParams();
            const CORE::CString password = "correct horse battery staple";
            CORE::CString record;
            ASSERT_TRUE( CPasswordHash::MakeRecord( password, cheap, record ) );
            ASSERT_TRUE( record.StartsWith( "$argon2id$m=1024,t=1,p=1$" ) );
            ASSERT_FALSE( Contains( record, password ) );
            bool upgrade = true;
            ASSERT_TRUE( CPasswordHash::Verify( password, record, cheap, upgrade ) && !upgrade );
            ASSERT_FALSE( CPasswordHash::Verify( "correct horse battery stapler", record, cheap, upgrade ) );
            ASSERT_FALSE( CPasswordHash::Verify( CORE::CString::Empty, record, cheap, upgrade ) );
            CORE::CString again;
            ASSERT_TRUE( CPasswordHash::MakeRecord( password, cheap, again ) && again != record );

            // the record keeps its parameters: raised parameters still verify it and ask for a new one
            SPasswordParams stronger = cheap;
            stronger.memoryInKiB = 2048;
            stronger.iterations = 2;
            ASSERT_TRUE( CPasswordHash::Verify( password, record, stronger, upgrade ) && upgrade );
            SPasswordParams parsed;
            CORE::CDynamicBuffer salt, hash;
            ASSERT_TRUE( CPasswordHash::ParseRecord( record, parsed, salt, hash ) );
            ASSERT_TRUE( parsed.algorithm == "argon2id" && 1024 == parsed.memoryInKiB && 1 == parsed.iterations && 1 == parsed.lanes );
            ASSERT_TRUE( CPasswordHash::SaltSize == salt.GetDataSize() && CPasswordHash::HashSize == hash.GetDataSize() );

            // the fallback, verified whatever is current, and upgraded to it
            SPasswordParams pbkdf2;
            pbkdf2.algorithm = CPasswordHash::Pbkdf2Sha256;
            pbkdf2.iterations = 1000;
            CORE::CString pbkdf2Record;
            ASSERT_TRUE( CPasswordHash::MakeRecord( password, pbkdf2, pbkdf2Record ) && pbkdf2Record.StartsWith( "$pbkdf2-sha256$i=1000$" ) );
            ASSERT_TRUE( CPasswordHash::Verify( password, pbkdf2Record, cheap, upgrade ) && upgrade );
            ASSERT_FALSE( CPasswordHash::Verify( "other", pbkdf2Record, cheap, upgrade ) );

            // the dummy record of the user names that do not exist: the same cost, no password matches
            const CORE::CString dummy = CPasswordHash::MakeDummyRecord( cheap );
            ASSERT_TRUE( CPasswordHash::ParseRecord( dummy, parsed, salt, hash ) && parsed.memoryInKiB == cheap.memoryInKiB && parsed.iterations == cheap.iterations );
            ASSERT_FALSE( CPasswordHash::Verify( password, dummy, cheap, upgrade ) );
            ASSERT_FALSE( CPasswordHash::Verify( CORE::CString::Empty, dummy, cheap, upgrade ) );

            // what is not a record
            ASSERT_FALSE( CPasswordHash::ParseRecord( "plain text", parsed, salt, hash ) );
            ASSERT_FALSE( CPasswordHash::ParseRecord( "$argon2id$m=1024$abc$def", parsed, salt, hash ) );
            ASSERT_FALSE( CPasswordHash::ParseRecord( "$md5$i=1$c2FsdA$aGFzaA", parsed, salt, hash ) );
            ASSERT_FALSE( CPasswordHash::Verify( password, "$argon2id$m=0,t=1,p=1$AAAAAAAAAAAAAAAAAAAAAA$AAAA", cheap, upgrade ) );

            CORE::CString wiped = "secret text";
            CPasswordHash::Wipe( wiped );
            ASSERT_TRUE( wiped.IsNULLOrEmpty() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: users file: owner only, replaced atomically, a change by another process is seen and named, a file others can read is refused" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_WebUsers" );
            const CORE::CString stateDir = CORE::CombinePath( dir.GetPath(), "state" );
            CStore store( stateDir );
            CWebUserStore service( store );
            CORE::CString problem;
            ASSERT_TRUE( service.Load( problem ) && 0 == service.GetCount() );

            SWebUser alice;
            alice.name = "alice";
            alice.role = WebRoleAdmin;
            alice.createdInUnixSecs = 1700000000;
            ASSERT_TRUE( CPasswordHash::MakeRecord( "alice's long password", CheapParams(), alice.record ) );
            ASSERT_TRUE( service.Add( alice, problem ) );
            ASSERT_FALSE( service.Add( alice, problem ) );
            SWebUser invalid = alice;
            invalid.name = "no spaces please";
            ASSERT_FALSE( service.Add( invalid, problem ) );
            invalid = alice;
            invalid.role = "root";
            ASSERT_FALSE( service.Add( invalid, problem ) );

            const CORE::CString path = store.GetPath( "users" );
            struct stat before;
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &before ) && 0600 == ( before.st_mode & 0777 ) );
            const CORE::CString text = ReadText( path );
            ASSERT_TRUE( Contains( text, "alice\tadmin\tenabled\t1700000000\t$argon2id$" ) );
            ASSERT_FALSE( Contains( text, "alice's long password" ) );

            // the command line is another process with a store of its own on the same directory
            CStore otherStore( stateDir );
            CWebUserStore commandLine( otherStore );
            ASSERT_TRUE( commandLine.Load( problem ) && 1 == commandLine.GetCount() );
            CORE::CString newRecord;
            ASSERT_TRUE( CPasswordHash::MakeRecord( "alice's new password", CheapParams(), newRecord ) );
            ASSERT_TRUE( commandLine.SetRecord( "alice", newRecord, problem ) );

            // replaced, not written over: a new file moved over the old one, with the same mode, and nothing left beside it
            struct stat after;
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &after ) && 0600 == ( after.st_mode & 0777 ) );
            ASSERT_TRUE( before.st_ino != after.st_ino );
            #if defined( __linux__ )
            UInt32 files = 0;
            DIR* listing = ::opendir( stateDir.C_String() );
            ASSERT_TRUE( GUCEF_NULL != listing );
            struct dirent* item = GUCEF_NULL;
            while ( GUCEF_NULL != ( item = ::readdir( listing ) ) )
                if ( '.' != item->d_name[ 0 ] )
                    ++files;
            ::closedir( listing );
            ASSERT_TRUE( 1 == files );
            #endif

            // the service sees it on its next read, and names the user whose sessions end
            UInt64 now = 100000;
            CORE::CString::StringVector changed;
            ASSERT_TRUE( service.Refresh( now, 1000, changed ) && 1 == changed.size() && changed[ 0 ] == "alice" );
            SWebUser found;
            ASSERT_TRUE( service.Find( "alice", found ) && found.record == newRecord );
            changed.clear();
            ASSERT_TRUE( service.Refresh( now + 1500, 1000, changed ) && changed.empty() );

            ASSERT_TRUE( commandLine.SetDisabled( "alice", true, problem ) );
            changed.clear();
            ASSERT_TRUE( service.Refresh( now + 3000, 1000, changed ) && 1 == changed.size() );
            ASSERT_TRUE( service.Find( "alice", found ) && found.disabled );

            // within the interval the file is not read again
            SWebUser bob = alice;
            bob.name = "bob";
            bob.role = WebRoleViewer;
            ASSERT_TRUE( commandLine.Add( bob, problem ) );
            changed.clear();
            ASSERT_TRUE( service.Refresh( now + 3500, 1000, changed ) && !service.Find( "bob", found ) );
            ASSERT_TRUE( service.Refresh( now + 4100, 1000, changed ) && service.Find( "bob", found ) && changed.empty() );
            ASSERT_TRUE( commandLine.Remove( "bob", problem ) );
            changed.clear();
            ASSERT_TRUE( service.Refresh( now + 5200, 1000, changed ) && 1 == changed.size() && changed[ 0 ] == "bob" );

            // an upgrade of the hash keeps the sessions, and does not take the place of a password changed meanwhile
            ASSERT_TRUE( commandLine.SetDisabled( "alice", false, problem ) );
            service.Refresh( now + 6300, 1000, changed );
            changed.clear();
            CORE::CString upgraded;
            ASSERT_TRUE( CPasswordHash::MakeRecord( "alice's new password", CheapParams(), upgraded ) );
            ASSERT_TRUE( service.SetRecord( "alice", upgraded, problem, &newRecord ) );
            ASSERT_TRUE( service.Refresh( now + 6400, 1000, changed ) && changed.empty() );
            ASSERT_FALSE( service.SetRecord( "alice", newRecord, problem, &newRecord ) );

            // a change made through the service itself ends the sessions as well
            ASSERT_TRUE( service.SetRecord( "alice", newRecord, problem ) );
            changed.clear();
            ASSERT_TRUE( service.Refresh( now + 6500, 1000, changed ) && 1 == changed.size() );

            // a file others can read is refused, and what was known stays
            ::chmod( path.C_String(), 0644 );
            ASSERT_FALSE( service.Refresh( now + 8000, 1000, changed ) );
            ASSERT_TRUE( service.Find( "alice", found ) );
            ::chmod( path.C_String(), 0600 );

            // lines that are not a user are skipped
            GUCEF::vector< SWebUser > parsed;
            CWebUserStore::Parse( "# a comment\nbad line\nx y\tadmin\tenabled\t0\t$r\ncarol\tviewer\tdisabled\t5\t$r\n", parsed );
            ASSERT_TRUE( 1 == parsed.size() && parsed[ 0 ].name == "carol" && parsed[ 0 ].disabled && 5 == parsed[ 0 ].createdInUnixSecs );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: login: no credentials, a bad token, a bad password, an unknown user at the same cost, the cookie, the CSRF header, viewer and admin, logout" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CWebFixture lab( "gucefLabLink_TestApp_WebLogin" );
            ASSERT_TRUE( lab.AddUser( "alice", WebRoleAdmin, "alice-long-password" ) );
            ASSERT_TRUE( lab.AddUser( "bob", WebRoleViewer, "bob-long-password" ) );
            ASSERT_TRUE( lab.AddUser( "carol", WebRoleAdmin, "carol-long-password", true ) );
            CLabWebApi api( lab.settings, lab.ranges, lab.hub, lab.users, false, "the-token-of-the-site-0123456789" );
            api.SetListening( "127.0.0.1", 47813 );
            UInt64 now = 1000000;
            WEB::CHttpResponseData response;
            bool hashed = false;

            // the page is for everyone, its data is not
            ASSERT_TRUE( 200 == Ask( api, Request( "GET", "/" ), "127.0.0.1", now, response ) && Contains( response.contentType, "text/html" ) );
            ASSERT_TRUE( Contains( BodyOf( response ), "<title>Lab hub</title>" ) && !HeaderOf( response, "Content-Security-Policy" ).IsNULLOrEmpty() );
            ASSERT_TRUE( 200 == Ask( api, Request( "GET", "/app.js" ), "127.0.0.1", now, response ) && Contains( response.contentType, "javascript" ) );
            ASSERT_TRUE( 401 == Ask( api, Request( "GET", "/api/v1/nodes" ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( 401 == Ask( api, Request( "GET", "/api/v1/session" ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( 404 == Ask( api, Request( "GET", "/nothing-here" ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( 405 == Ask( api, Request( "GET", "/api/v1/login" ), "127.0.0.1", now, response ) );

            // a POST without the CSRF header, or not a form, is refused before anything else
            ASSERT_TRUE( 403 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ), false ), "127.0.0.1", now, response, &hashed ) && !hashed );
            WEB::CHttpRequestData json = Request( "POST", "/api/v1/login", "{\"username\":\"alice\"}" );
            json.contentRepresentation = "application/json";
            ASSERT_TRUE( 415 == Ask( api, json, "127.0.0.1", now, response, &hashed ) && !hashed );

            // the same answer for a wrong password, a user that is not there and one that is disabled; each costs a hash
            ASSERT_TRUE( 401 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "not-her-password" ) ), "127.0.0.1", now, response, &hashed ) && hashed );
            const CORE::CString refused = BodyOf( response );
            ASSERT_TRUE( Contains( refused, "The user name or password is not correct." ) && HeaderOf( response, "Set-Cookie" ).IsNULLOrEmpty() );
            SPasswordJob unknownJob;
            ASSERT_TRUE( 401 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "mallory", "whatever-it-is" ) ), "127.0.0.1", now, response, &hashed, &unknownJob ) && hashed );
            ASSERT_TRUE( BodyOf( response ) == refused );
            SPasswordJob disabledJob;
            ASSERT_TRUE( 401 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "carol", "carol-long-password" ) ), "127.0.0.1", now, response, &hashed, &disabledJob ) && hashed );
            ASSERT_TRUE( BodyOf( response ) == refused );
            SWebUser alice;
            ASSERT_TRUE( lab.users.Find( "alice", alice ) );
            SPasswordParams real, dummy;
            CORE::CDynamicBuffer salt, hash;
            ASSERT_TRUE( CPasswordHash::ParseRecord( alice.record, real, salt, hash ) && CPasswordHash::ParseRecord( unknownJob.record, dummy, salt, hash ) );
            ASSERT_TRUE( real.algorithm == dummy.algorithm && real.memoryInKiB == dummy.memoryInKiB && real.iterations == dummy.iterations && real.lanes == dummy.lanes );
            ASSERT_TRUE( !unknownJob.knownUser && !disabledJob.knownUser && unknownJob.record == disabledJob.record );
            ASSERT_TRUE( 400 == Ask( api, Request( "POST", "/api/v1/login", "username=alice&password=" ), "127.0.0.1", now, response ) );

            // the token: as a login and as a bearer header
            ASSERT_TRUE( 401 == Ask( api, Request( "POST", "/api/v1/login", "token=not-the-token" ), "127.0.0.1", now, response ) );
            WEB::CHttpRequestData bearer = Request( "GET", "/api/v1/nodes" );
            bearer.otherHeaders[ "authorization" ] = "Bearer not-the-token";
            ASSERT_TRUE( 401 == Ask( api, bearer, "127.0.0.1", now, response ) );
            bearer.otherHeaders[ "authorization" ] = "Bearer the-token-of-the-site-0123456789";
            ASSERT_TRUE( 200 == Ask( api, bearer, "127.0.0.1", now, response ) && Contains( BodyOf( response ), "\"nodes\":[" ) );
            bearer.requestUri = "/api/v1/users";
            ASSERT_TRUE( 200 == Ask( api, bearer, "127.0.0.1", now, response ) && Contains( BodyOf( response ), "\"name\":\"alice\"" ) );
            ASSERT_FALSE( Contains( BodyOf( response ), "$argon2id" ) );
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", "token=the-token-of-the-site-0123456789" ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( Contains( BodyOf( response ), "\"viaToken\":true" ) && 64 == SessionIdOf( response ).Length() );

            // a password login: the cookie holds a session id only, HttpOnly, SameSite=Strict, not Secure over plain HTTP
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "127.0.0.1", now, response ) );
            const CORE::CString cookie = HeaderOf( response, "Set-Cookie" );
            const CORE::CString aliceSession = SessionIdOf( response );
            ASSERT_TRUE( 64 == aliceSession.Length() && cookie == "lablink-session=" + aliceSession + "; Path=/; HttpOnly; SameSite=Strict" );
            ASSERT_TRUE( Contains( BodyOf( response ), "\"user\":\"alice\"" ) && Contains( BodyOf( response ), "\"role\":\"admin\"" ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/session" ), aliceSession, now, response ) && Contains( BodyOf( response ), "\"user\":\"alice\"" ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/nodes" ), aliceSession, now, response ) );
            ASSERT_TRUE( 401 == AskWith( api, Request( "GET", "/api/v1/nodes" ), aliceSession.SubstrToIndex( 63, true ) + "0", now, response ) );
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password", true ) ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( Contains( HeaderOf( response, "Set-Cookie" ), "; Max-Age=2592000" ) );

            // a viewer reads, an admin manages the users
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "bob", "bob-long-password" ) ), "127.0.0.1", now, response ) );
            const CORE::CString bobSession = SessionIdOf( response );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/nodes" ), bobSession, now, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/endpoints" ), bobSession, now, response ) );
            ASSERT_TRUE( 403 == AskWith( api, Request( "GET", "/api/v1/users" ), bobSession, now, response ) );
            ASSERT_TRUE( 403 == AskWith( api, Request( "POST", "/api/v1/users/disable", "name=alice" ), bobSession, now, response ) );
            ASSERT_TRUE( 403 == AskWith( api, Request( "POST", "/api/v1/users/enable", "name=bob", false ), aliceSession, now, response ) );
            ASSERT_TRUE( 409 == AskWith( api, Request( "POST", "/api/v1/users/disable", "name=alice" ), aliceSession, now, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "POST", "/api/v1/users/disable", "name=bob" ), aliceSession, now, response ) );
            ASSERT_TRUE( 401 == AskWith( api, Request( "GET", "/api/v1/nodes" ), bobSession, now, response ) );
            ASSERT_TRUE( 401 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "bob", "bob-long-password" ) ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "POST", "/api/v1/users/enable", "name=bob" ), aliceSession, now, response ) );
            ASSERT_TRUE( 404 == AskWith( api, Request( "POST", "/api/v1/users/enable", "name=nobody" ), aliceSession, now, response ) );

            // an admin adds a user: the password is hashed as a job, with the minimum length
            ASSERT_TRUE( 400 == AskWith( api, Request( "POST", "/api/v1/users/add", "name=dave&role=viewer&password=short" ), aliceSession, now, response ) );
            ASSERT_TRUE( 400 == AskWith( api, Request( "POST", "/api/v1/users/add", "name=dave&role=owner&password=dave-long-password" ), aliceSession, now, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "POST", "/api/v1/users/add", "name=dave&role=viewer&password=dave-long-password" ), aliceSession, now, response ) );
            ASSERT_TRUE( 409 == AskWith( api, Request( "POST", "/api/v1/users/add", "name=dave&role=viewer&password=dave-long-password" ), aliceSession, now, response ) );
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "dave", "dave-long-password" ) ), "127.0.0.1", now, response ) );
            ASSERT_TRUE( Contains( BodyOf( response ), "\"role\":\"viewer\"" ) );

            // logout ends the session on the hub, and clears the cookie
            ASSERT_TRUE( 403 == AskWith( api, Request( "POST", "/api/v1/logout", CORE::CString::Empty, false ), aliceSession, now, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "POST", "/api/v1/logout" ), aliceSession, now, response ) );
            ASSERT_TRUE( HeaderOf( response, "Set-Cookie" ) == "lablink-session=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0" );
            ASSERT_TRUE( 401 == AskWith( api, Request( "GET", "/api/v1/session" ), aliceSession, now, response ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: back-off: failed logins wait longer each time up to a bound, never for good, per address and per user, in a bounded table" )
        try
        {
            CLoginThrottle throttle( 4, 2, 1000, 8000, 60000 );
            UInt64 now = 0;
            throttle.OnFailure( "k", now );
            throttle.OnFailure( "k", now );
            ASSERT_TRUE( 0 == throttle.GetWaitInMs( "k", now ) );
            const UInt32 expected[] = { 1000, 2000, 4000, 8000, 8000, 8000 };
            for ( size_t i=0; i<sizeof( expected ) / sizeof( expected[ 0 ] ); ++i )
            {
                throttle.OnFailure( "k", now );
                ASSERT_TRUE( expected[ i ] == throttle.GetWaitInMs( "k", now ) );
            }
            ASSERT_TRUE( 0 == throttle.GetWaitInMs( "k", now + 8000 ) );
            throttle.OnSuccess( "k" );
            ASSERT_TRUE( 0 == throttle.GetFailures( "k" ) );
            for ( UInt32 i=0; i<10; ++i )
                throttle.OnFailure( "key" + CORE::ToString( i ), now + i );
            ASSERT_TRUE( 4 == throttle.GetCount() && 0 == throttle.GetFailures( "key0" ) && 1 == throttle.GetFailures( "key9" ) );
            throttle.Sweep( now + 100000 );
            ASSERT_TRUE( 0 == throttle.GetCount() );

            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CWebFixture lab( "gucefLabLink_TestApp_WebBackoff" );
            lab.settings.maxAddressDelayInSecs = 60;
            lab.settings.maxUserDelayInSecs = 4;
            ASSERT_TRUE( lab.AddUser( "alice", WebRoleAdmin, "alice-long-password" ) );
            CLabWebApi api( lab.settings, lab.ranges, lab.hub, lab.users, false, CORE::CString::Empty );
            WEB::CHttpResponseData response;
            now = 5000000;

            // one address: five failures are free, then it waits, the wait doubles and stays within the bound
            int status = 0;
            UInt32 attempts = 0;
            while ( 429 != status && attempts < 20 )
            {
                status = Ask( api, Request( "POST", "/api/v1/login", LoginBody( "nobody" + CORE::ToString( attempts ), "wrong-password" ) ), "10.0.0.66", now, response );
                ++attempts;
            }
            ASSERT_TRUE( 429 == status && 7 == attempts );
            ASSERT_TRUE( 1 == CORE::StringToUInt32( HeaderOf( response, "Retry-After" ) ) );
            for ( UInt32 i=0; i<20; ++i )
            {
                now += 61000;
                Ask( api, Request( "POST", "/api/v1/login", LoginBody( "nobody", "wrong-password" ) ), "10.0.0.66", now, response );
            }
            ASSERT_TRUE( api.GetAddressThrottle().GetWaitInMs( "ip:10.0.0.66", now ) <= 60000 );
            // another address is not held up by it
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "10.0.0.67", now, response ) );

            // one user from many addresses: a short wait only, so nobody can keep alice out
            status = 0;
            for ( UInt32 i=0; i<10 && 429 != status; ++i )
                status = Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "wrong-password" ) ), "10.0.1." + CORE::ToString( i ), now, response );
            ASSERT_TRUE( 429 == status && CORE::StringToUInt32( HeaderOf( response, "Retry-After" ) ) <= 4 );
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "10.0.2.1", now + 4000, response ) );
            ASSERT_TRUE( 0 == api.GetUserThrottle().GetFailures( "user:alice" ) );

            // many addresses and names: the tables stay within their bound
            for ( UInt32 i=0; i<3000; ++i )
                api.GetAddressThrottle().OnFailure( "ip:192.168." + CORE::ToString( i / 250 ) + "." + CORE::ToString( i % 250 ), now );
            ASSERT_TRUE( api.GetAddressThrottle().GetCount() <= lab.settings.maxThrottleEntries );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: sessions: idle and absolute end, revocation on a password change, the bound of the table, the cookie over TLS" )
        try
        {
            CWebSessionTable table( 3, 2 );
            CORE::CString first, second;
            ASSERT_TRUE( table.Create( "alice", false, false, 0, 1000, 5000, first ) && 64 == first.Length() );
            ASSERT_TRUE( table.Create( "alice", false, false, 0, 10000, 5000, second ) && first != second );
            SWebSession session;
            ASSERT_TRUE( table.Use( first, 900, session ) && table.Use( first, 1800, session ) );
            ASSERT_FALSE( table.Use( first, 2801, session ) );
            ASSERT_TRUE( table.Use( second, 4999, session ) );
            ASSERT_FALSE( table.Use( second, 5000, session ) );
            ASSERT_TRUE( 0 == table.GetCount() );

            CORE::CString ids[ 4 ];
            for ( int i=0; i<3; ++i )
                ASSERT_TRUE( table.Create( "alice", false, false, (UInt64) i, 100000, 100000, ids[ i ] ) );
            ASSERT_TRUE( 2 == table.GetCountOfUser( "alice" ) && !table.Use( ids[ 0 ], 10, session ) );
            ASSERT_TRUE( table.Create( "bob", false, false, 20, 100000, 100000, ids[ 3 ] ) );
            CORE::CString carol;
            ASSERT_TRUE( table.Create( "carol", false, false, 30, 100000, 100000, carol ) && 3 == table.GetCount() );
            ASSERT_TRUE( 1 == table.RemoveUser( "bob" ) && 2 == table.GetCount() );
            ASSERT_TRUE( 2 == table.Sweep( 200001 ) && 0 == table.GetCount() );

            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CWebFixture lab( "gucefLabLink_TestApp_WebSessions" );
            lab.settings.sessionIdleTimeoutInSecs = 10;
            lab.settings.sessionLifetimeInSecs = 30;
            lab.settings.maxSessions = 4;
            lab.settings.maxSessionsPerUser = 3;
            ASSERT_TRUE( lab.AddUser( "alice", WebRoleAdmin, "alice-long-password" ) );
            ASSERT_TRUE( lab.AddUser( "bob", WebRoleViewer, "bob-long-password" ) );
            CLabWebApi api( lab.settings, lab.ranges, lab.hub, lab.users, false, CORE::CString::Empty );
            WEB::CHttpResponseData response;
            UInt64 now = 9000000;

            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "127.0.0.1", now, response ) );
            CORE::CString id = SessionIdOf( response );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/session" ), id, now + 9000, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/session" ), id, now + 18000, response ) );
            ASSERT_TRUE( 200 == AskWith( api, Request( "GET", "/api/v1/session" ), id, now + 27000, response ) );
            ASSERT_TRUE( 401 == AskWith( api, Request( "GET", "/api/v1/session" ), id, now + 30000, response ) );
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "127.0.0.1", now, response ) );
            id = SessionIdOf( response );
            ASSERT_TRUE( 401 == AskWith( api, Request( "GET", "/api/v1/session" ), id, now + 10001, response ) );

            // a new password, given on the command line of the hub, ends the sessions there are
            now += 100000;
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "127.0.0.1", now, response ) );
            id = SessionIdOf( response );
            CStore otherStore( lab.store.GetStateDir() );
            CWebUserStore commandLine( otherStore );
            CORE::CString problem, record;
            ASSERT_TRUE( commandLine.Load( problem ) && CPasswordHash::MakeRecord( "alice-other-password", CheapParams(), record ) );
            ASSERT_TRUE( commandLine.SetRecord( "alice", record, problem ) );
            ASSERT_TRUE( 401 == AskWith( api, Request( "GET", "/api/v1/session" ), id, now + 2000, response ) );
            ASSERT_TRUE( 401 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-long-password" ) ), "127.0.0.1", now + 2000, response ) );
            ASSERT_TRUE( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-other-password" ) ), "127.0.0.1", now + 2000, response ) );

            // however often one logs in, the table keeps its bound
            UInt32 made = 0;
            for ( int i=0; i<12; ++i )
            {
                const CORE::CString peer = "10.9.0." + CORE::ToString( (UInt32) i );
                if ( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-other-password" ) ), peer, now + 3000 + i, response ) )
                    ++made;
                if ( 200 == Ask( api, Request( "POST", "/api/v1/login", LoginBody( "bob", "bob-long-password" ) ), peer, now + 3000 + i, response ) )
                    ++made;
            }
            ASSERT_TRUE( 24 == made && 4 == api.GetSessions().GetCount() && api.GetSessions().GetCountOfUser( "alice" ) <= 3 );

            // over TLS the cookie is Secure and has the __Host- prefix
            CLabWebApi tlsApi( lab.settings, lab.ranges, lab.hub, lab.users, true, CORE::CString::Empty );
            ASSERT_TRUE( 200 == Ask( tlsApi, Request( "POST", "/api/v1/login", LoginBody( "alice", "alice-other-password" ) ), "10.0.0.5", now, response ) );
            ASSERT_TRUE( HeaderOf( response, "Set-Cookie" ) == "__Host-lablink-session=" + SessionIdOf( response ) + "; Path=/; HttpOnly; SameSite=Strict; Secure" );
            ASSERT_TRUE( CLabWebApi::MakeCookie( "c", "v", true, 60, false ) == "c=v; Path=/; HttpOnly; SameSite=Strict; Secure; Max-Age=60" );
            ASSERT_TRUE( CLabWebApi::MakeCookie( "c", "v", false, 0, true ) == "c=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0" );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: the Host on plain HTTP over loopback, the allowed ranges, a node id in the path" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CWebFixture lab( "gucefLabLink_TestApp_WebHost" );
            CORE::CString problem;
            CORE::CString::StringVector list;
            list.push_back( "10.0.0.0/8" );
            list.push_back( "127.0.0.0/8" );
            ASSERT_TRUE( lab.ranges.Set( list, problem ) );
            CLabWebApi api( lab.settings, lab.ranges, lab.hub, lab.users, false, "the-token-of-the-site-0123456789" );
            api.SetListening( CORE::CString::Empty, 47813 );
            WEB::CHttpResponseData response;
            const UInt64 now = 1000;

            ASSERT_TRUE( 403 == Ask( api, Request( "GET", "/" ), "192.168.1.5", now, response ) );
            ASSERT_TRUE( 200 == Ask( api, Request( "GET", "/" ), "10.1.2.3", now, response ) );
            WEB::CHttpRequestData rebound = Request( "GET", "/" );
            rebound.requestHost = "evil.example:47813";
            ASSERT_TRUE( 403 == Ask( api, rebound, "127.0.0.1", now, response ) );
            ASSERT_TRUE( 200 == Ask( api, rebound, "10.1.2.3", now, response ) );     // a LAN peer names the hub as it knows it
            rebound.requestHost = "localhost:47813";
            ASSERT_TRUE( 200 == Ask( api, rebound, "127.0.0.1", now, response ) );
            rebound.requestHost = "127.0.0.1:1";
            ASSERT_TRUE( 403 == Ask( api, rebound, "127.0.0.1", now, response ) );
            CLabWebApi tlsApi( lab.settings, lab.ranges, lab.hub, lab.users, true, CORE::CString::Empty );
            rebound.requestHost = "evil.example:47813";
            ASSERT_TRUE( 200 == Ask( tlsApi, rebound, "127.0.0.1", now, response ) );

            // the router of the server is not told the address: a session or the token is needed
            WEB::CHttpRequestData node = Request( "GET", "/api/v1/nodes/abc" );
            SPasswordJob job;
            ASSERT_TRUE( api.Handle( node, GUCEF_NULL, now, response, job ) && 401 == response.statusCode );
            node.otherHeaders[ "authorization" ] = "Bearer the-token-of-the-site-0123456789";
            ASSERT_TRUE( api.Handle( node, GUCEF_NULL, now, response, job ) && 404 == response.statusCode );
            node.requestUri = "/api/v1/nodes/a%2Fb";
            ASSERT_TRUE( api.Handle( node, GUCEF_NULL, now, response, job ) && 404 == response.statusCode );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: the JSON of the nodes and endpoints against a populated registry: order, status, fields, escaping" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CWebFixture lab( "gucefLabLink_TestApp_WebJson" );
            const UInt64 mono = 50000;
            const UInt64 unixNow = 1800000000;

            // known from storage, no report since the hub started
            SNodeEntry stored;
            stored.report.machineId = "c0000000000000000000000000000003";
            stored.report.name = "charlie";
            stored.verified = true;
            stored.firstSeenInUnixSecs = unixNow - 9000;
            stored.lastSeenInUnixSecs = unixNow - 8000;
            GUCEF::vector< SNodeEntry > storedEntries;
            storedEntries.push_back( stored );

            // online, with everything a report can hold
            SNodeReport a;
            a.machineId = "a0000000000000000000000000000001";
            a.instanceId = "run";
            a.startedInUnixSecs = 1;
            a.name = "alpha \"one\"";
            a.os = "linux";
            a.arch = "x86_64";
            a.agentVersion = "gucefLabLink 1.1";
            a.nodeState = NodeStateBusy;
            a.cpuCores = 16;
            a.memTotalInMiB = 65536;
            SGpuInfo gpu;
            gpu.name = "RTX 5080";
            gpu.vendor = "nvidia";
            gpu.vramTotalInMiB = 16384;
            gpu.vramFreeInMiB = 12000;
            a.gpus.push_back( gpu );
            SHostInfo host;
            host.name = "native";
            host.state = "ready";
            a.hosts.push_back( host );
            SWorkloadInfo llm;
            llm.id = "llm-1";
            llm.kind = "llm";
            llm.state = "running";
            llm.endpoint = "http://10.0.0.5:8080";
            llm.model = "qwen";
            a.workloads.push_back( llm );
            SWorkloadInfo stopped = llm;
            stopped.id = "llm-2";
            stopped.state = "stopped";
            stopped.endpoint = "http://10.0.0.5:8081";
            a.workloads.push_back( stopped );
            a.capabilities.push_back( "llm" );
            a.capabilities.push_back( "mcp" );
            ASSERT_TRUE( REPORT_ACCEPTED == lab.registry.OnReport( a, true, "10.0.0.5", mono, unixNow ) );

            // reported with the lab secret, then silent: offline and unverified
            SNodeReport b;
            b.machineId = "b0000000000000000000000000000002";
            b.instanceId = "run";
            b.name = "bravo";
            SWorkloadInfo gone = llm;
            gone.endpoint = "http://10.0.0.6:8080";
            b.workloads.push_back( gone );
            ASSERT_TRUE( REPORT_ACCEPTED == lab.registry.OnReport( b, false, "10.0.0.6", mono - 70000 + 100000, unixNow - 500 ) );
            lab.registry.Evaluate( mono - 70000 + 100000 + 70000, unixNow );
            ASSERT_TRUE( REPORT_ACCEPTED == lab.registry.OnReport( a, true, "10.0.0.5", mono - 70000 + 100000 + 70000, unixNow ) );
            lab.registry.Load( storedEntries, mono - 70000 + 100000 + 70000 );
            SNodeEntry entry;
            ASSERT_TRUE( lab.registry.Find( b.machineId, entry ) && NODELIVENESS_OFFLINE == entry.liveness );

            CLabWebApi api( lab.settings, lab.ranges, lab.hub, lab.users, false, "the-token-of-the-site-0123456789" );
            WEB::CHttpRequestData request = Request( "GET", "/api/v1/nodes" );
            request.otherHeaders[ "authorization" ] = "Bearer the-token-of-the-site-0123456789";
            WEB::CHttpResponseData response;
            ASSERT_TRUE( 200 == Ask( api, request, "127.0.0.1", 1, response ) && Contains( response.contentType, "application/json" ) );
            ASSERT_TRUE( HeaderOf( response, "X-Content-Type-Options" ) == "nosniff" && response.cacheability == "no-store" );
            const CORE::CString json = BodyOf( response );
            ASSERT_TRUE( json.StartsWith( "{\"hub\":\"testhub\",\"hubTime\":" ) );
            const Int32 alpha = json.HasSubstr( "\"name\":\"alpha \\\"one\\\"\"" );
            const Int32 charlie = json.HasSubstr( "\"name\":\"charlie\"" );
            const Int32 bravo = json.HasSubstr( "\"name\":\"bravo\"" );
            ASSERT_TRUE( alpha > 0 && charlie > alpha && bravo > charlie );
            ASSERT_TRUE( Contains( json, "\"machineId\":\"a0000000000000000000000000000001\",\"name\":\"alpha \\\"one\\\"\",\"os\":\"linux\",\"arch\":\"x86_64\","
                                         "\"agentVersion\":\"gucefLabLink 1.1\",\"status\":\"online\",\"verified\":true,\"address\":\"10.0.0.5\"" ) );
            ASSERT_TRUE( Contains( json, "\"nodeState\":\"busy\"" ) && Contains( json, "\"cpuCores\":16" ) && Contains( json, "\"memTotalMiB\":65536" ) );
            ASSERT_TRUE( Contains( json, "\"gpus\":[{\"name\":\"RTX 5080\",\"vendor\":\"nvidia\",\"vramTotalMiB\":16384,\"vramFreeMiB\":12000,\"utilizationPercent\":0}]" ) );
            ASSERT_TRUE( Contains( json, "\"hosts\":[{\"name\":\"native\",\"state\":\"ready\",\"detail\":\"\"}]" ) );
            ASSERT_TRUE( Contains( json, "\"capabilities\":[\"llm\",\"mcp\"]" ) );
            ASSERT_TRUE( Contains( json, "\"machineId\":\"b0000000000000000000000000000002\",\"name\":\"bravo\"" ) && Contains( json, "\"status\":\"offline\",\"verified\":false" ) );
            ASSERT_TRUE( Contains( json, "\"status\":\"unknown\",\"verified\":true" ) && Contains( json, "\"lastSeen\":" + CORE::ToString( unixNow - 8000 ) ) );

            request.requestUri = "/api/v1/nodes/a0000000000000000000000000000001";
            ASSERT_TRUE( 200 == Ask( api, request, "127.0.0.1", 1, response ) && BodyOf( response ).StartsWith( "{\"machineId\":\"a0000000000000000000000000000001\"" ) );
            request.requestUri = "/api/v1/nodes/ffff";
            ASSERT_TRUE( 404 == Ask( api, request, "127.0.0.1", 1, response ) );

            // the endpoints: what runs on the nodes that are online
            request.requestUri = "/api/v1/endpoints";
            ASSERT_TRUE( 200 == Ask( api, request, "127.0.0.1", 1, response ) );
            const CORE::CString endpoints = BodyOf( response );
            ASSERT_TRUE( Contains( endpoints, "\"endpoints\":[{\"kind\":\"llm\",\"machineId\":\"a0000000000000000000000000000001\",\"node\":\"alpha \\\"one\\\"\",\"url\":\"http://10.0.0.5:8080\",\"model\":\"qwen\",\"workloadId\":\"llm-1\",\"idleSlots\":0}]" ) );
            ASSERT_FALSE( Contains( endpoints, "8081" ) || Contains( endpoints, "10.0.0.6" ) );

            // with no node at all the lists are empty, not missing
            CNodeRegistry empty;
            CRegistryHubView emptyHub( empty, "emptyhub" );
            CLabWebApi emptyApi( lab.settings, lab.ranges, emptyHub, lab.users, false, "the-token-of-the-site-0123456789" );
            request.requestUri = "/api/v1/endpoints";
            ASSERT_TRUE( 200 == Ask( emptyApi, request, "127.0.0.1", 1, response ) && Contains( BodyOf( response ), "\"endpoints\":[]}" ) );
            request.requestUri = "/api/v1/nodes";
            ASSERT_TRUE( 200 == Ask( emptyApi, request, "127.0.0.1", 1, response ) && Contains( BodyOf( response ), "\"nodes\":[]}" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: no log line holds a password or a token" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLogCapture capture;
            CWebFixture lab( "gucefLabLink_TestApp_WebLog" );
            ASSERT_TRUE( lab.AddUser( "alice", WebRoleAdmin, "Alice-Secret-Password-1" ) );
            CLabWebApi api( lab.settings, lab.ranges, lab.hub, lab.users, false, "Token-Secret-0123456789abcdef" );
            WEB::CHttpResponseData response;
            UInt64 now = 1000;
            Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "Alice-Secret-Password-1" ) ), "127.0.0.1", now, response );
            const CORE::CString session = SessionIdOf( response );
            Ask( api, Request( "POST", "/api/v1/login", LoginBody( "alice", "Alice-Wrong-Password-2" ) ), "127.0.0.1", now, response );
            Ask( api, Request( "POST", "/api/v1/login", LoginBody( "Typed-Password-In-Name-3", "Mallory-Password-4" ) ), "127.0.0.1", now, response );
            Ask( api, Request( "POST", "/api/v1/login", "token=Token-Secret-0123456789abcdef" ), "127.0.0.1", now, response );
            Ask( api, Request( "POST", "/api/v1/login", "token=Token-Wrong-5" ), "127.0.0.1", now, response );
            AskWith( api, Request( "POST", "/api/v1/users/add", "name=bob&role=viewer&password=Bob-Secret-Password-6" ), session, now, response );
            AskWith( api, Request( "POST", "/api/v1/users/add", "name=carl&role=viewer&password=Short-7" ), session, now, response );
            AskWith( api, Request( "POST", "/api/v1/logout" ), session, now, response );
            const CORE::CString log = capture.GetText();
            ASSERT_TRUE( Contains( log, "login of alice" ) && Contains( log, "failed login for the user alice" ) && Contains( log, "added the user bob" ) );
            const char* secrets[] = { "Alice-Secret-Password-1", "Alice-Wrong-Password-2", "Typed-Password-In-Name-3", "Mallory-Password-4",
                                      "Token-Secret-0123456789abcdef", "Token-Wrong-5", "Bob-Secret-Password-6", "Short-7" };
            for ( size_t i=0; i<sizeof( secrets ) / sizeof( secrets[ 0 ] ); ++i )
                ASSERT_FALSE( Contains( log, secrets[ i ] ) );
            ASSERT_FALSE( session.IsNULLOrEmpty() || Contains( log, session ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: the cluster role serves the website only with --web, and runs on without it when it cannot start" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_WebCluster" );
            CStore store( CORE::CombinePath( dir.GetPath(), "state" ) );
            CORE::CString problem;
            ASSERT_TRUE( store.WriteSecret( MakeLabSecret(), problem ) );
            SSettings settings;
            settings.stateDir = store.GetStateDir();
            settings.clusterName = "testhub";
            settings.useMdns = false;
            settings.useBroadcast = false;
            settings.dbType = "none";
            settings.k3sBin = CORE::CombinePath( dir.GetPath(), "no-k3s" );
            settings.mcpDir = CORE::CombinePath( dir.GetPath(), "mcp" );
            settings.web.port = 0;

            for ( int round=0; round<3; ++round )
            {
                settings.labPort = TESTFW::CTlsTestServer::FindFreePort();
                settings.accessPort = TESTFW::CTlsTestServer::FindFreePort();
                settings.web.enabled = round > 0;
                settings.web.allowPlainHttp = round > 1;
                CClusterRole cluster( settings, store );
                ASSERT_TRUE( cluster.Start( problem ) );
                if ( round < 2 )
                {
                    // off, or without a certificate and without the plain HTTP opt-in: no website, the lab as before
                    ASSERT_TRUE( GUCEF_NULL == cluster.GetWebSite() );
                }
                else
                {
                    ASSERT_TRUE( GUCEF_NULL != cluster.GetWebSite() && cluster.GetWebSite()->GetPort() > 0 && !cluster.GetWebSite()->IsTls() );
                    ASSERT_TRUE( TESTFW::CTlsTestServer::CanConnect( cluster.GetWebSite()->GetPort() ) );
                }
                ASSERT_TRUE( TESTFW::CTlsTestServer::CanConnect( settings.labPort ) );
                cluster.Stop();
            }
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: the command line adds, changes, disables and lists users, with the password on standard input; the running hub follows" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            const CORE::CString program = CORE::RelativePath( "$MODULEDIR$/gucefLabLink" );
            GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( program ), "gucefLabLink is not next to the test application" );
            CWebFixture lab( "gucefLabLink_TestApp_WebCommands" );
            const CORE::CString stateDir = lab.store.GetStateDir();
            CLabWebApi service( lab.settings, lab.ranges, lab.hub, lab.users, false, CORE::CString::Empty );
            WEB::CHttpResponseData response;
            UInt64 now = 1000000;
            CORE::CString output;

            ASSERT_TRUE( 2 == RunWithPassword( program, stateDir, "a-good-long-password", "--add-user alice", output ) );
            ASSERT_TRUE( 0 != RunWithPassword( program, stateDir, "short", "--add-user alice --role admin", output ) && Contains( output, "a password has 12" ) );
            ASSERT_TRUE( 0 == RunWithPassword( program, stateDir, "a-good-long-password", "--add-user alice --role admin", output ) );
            ASSERT_FALSE( Contains( output, "a-good-long-password" ) );
            ASSERT_TRUE( 0 != RunWithPassword( program, stateDir, "a-good-long-password", "--add-user alice --role admin", output ) );

            CORE::CProcess::TStringVector args;
            args.push_back( "--state-dir" ); args.push_back( stateDir );
            args.push_back( "--list-users" );
            CORE::CProcess::TStringMap quiet;
            quiet[ "GUCEF_CONSOLE_LOGGER" ] = "none";
            ASSERT_TRUE( 0 == Run( program, args, quiet, output ) && Contains( output, "alice" ) && Contains( output, "admin" ) && Contains( output, "argon2id" ) );

            // the hub that runs meanwhile takes the user, with the record the command line made at the full cost
            ASSERT_TRUE( 200 == Ask( service, Request( "POST", "/api/v1/login", LoginBody( "alice", "a-good-long-password" ) ), "127.0.0.1", now, response ) );
            const CORE::CString session = SessionIdOf( response );
            ASSERT_TRUE( 0 == RunWithPassword( program, stateDir, "another-long-password", "--set-password alice", output ) );
            now += 2000;
            ASSERT_TRUE( 401 == AskWith( service, Request( "GET", "/api/v1/session" ), session, now, response ) );
            ASSERT_TRUE( 200 == Ask( service, Request( "POST", "/api/v1/login", LoginBody( "alice", "another-long-password" ) ), "127.0.0.1", now, response ) );

            // a login right after a disable, within the second the sessions wait for, is refused already
            args.back() = "--disable-user"; args.push_back( "alice" );
            ASSERT_TRUE( 0 == Run( program, args, quiet, output ) );
            ASSERT_TRUE( 401 == Ask( service, Request( "POST", "/api/v1/login", LoginBody( "alice", "another-long-password" ) ), "127.0.0.1", now, response ) );
            now += 2000;
            args[ args.size() - 2 ] = "--enable-user";
            ASSERT_TRUE( 0 == Run( program, args, quiet, output ) );
            args[ args.size() - 2 ] = "--remove-user";
            ASSERT_TRUE( 0 == Run( program, args, quiet, output ) );
            ASSERT_TRUE( 0 != Run( program, args, quiet, output ) );
            now += 2000;
            ASSERT_TRUE( 401 == Ask( service, Request( "POST", "/api/v1/login", LoginBody( "alice", "another-long-password" ) ), "127.0.0.1", now, response ) );

            // the user commands leave the files of the lab alone
            ASSERT_FALSE( CORE::FileExists( lab.store.GetPath( "approvals" ) ) || CORE::FileExists( lab.store.GetPath( "identity" ) ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: the hub as a program over plain HTTP on loopback: page, no credentials, token, login, cookie, logout, Host; no port without --web" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            GUCEF_TESTFW_SKIP_IF( !IsCurlAvailable(), "curl is not installed" );
            CHubProgram hub( "gucefLabLink_TestApp_WebHub" );
            GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( hub.program ), "gucefLabLink is not next to the test application" );
            CORE::CString output;
            ASSERT_TRUE( 0 == RunWithPassword( hub.program, hub.Path( "state" ), "alice-long-password", "--add-user alice --role admin", output ) );
            ASSERT_TRUE( WriteText( hub.Path( "password" ), "alice-long-password" ) );

            // without --web nothing listens on the port of the site
            const UInt16 webPort = TESTFW::CTlsTestServer::FindFreePort();
            CORE::CProcess::TStringVector extra;
            extra.push_back( "--web-port" ); extra.push_back( CORE::ToString( webPort ) );
            ASSERT_TRUE( hub.Start( extra ) );
            MT::ThreadDelay( 500 );
            ASSERT_FALSE( TESTFW::CTlsTestServer::CanConnect( webPort ) );
            hub.Stop();

            hub.labPort = TESTFW::CTlsTestServer::FindFreePort();
            hub.accessPort = TESTFW::CTlsTestServer::FindFreePort();
            extra.push_back( "--web" );
            extra.push_back( "--web-allow-plain-http" );
            extra.push_back( "--web-token-file" ); extra.push_back( hub.Path( "state/web-token" ) );
            ASSERT_TRUE( hub.Start( extra ) && WaitForPort( webPort, 30000 ) );
            const CORE::CString site = "http://127.0.0.1:" + CORE::ToString( webPort );
            CORE::CString body;
            CORE::CProcess::TStringVector args;
            args.push_back( site + "/" );
            ASSERT_TRUE( 200 == Curl( args, body ) && Contains( body, "<title>Lab hub</title>" ) );
            args.back() = site + "/api/v1/nodes";
            ASSERT_TRUE( 401 == Curl( args, body ) );

            // the token file was made for the owner, the token works as a bearer
            CORE::CDynamicBuffer tokenFile;
            CORE::CString problem;
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( hub.Path( "state/web-token" ), tokenFile, problem ) && tokenFile.GetDataSize() > 20 );
            const CORE::CString token = StripText( CORE::CString( static_cast< const char* >( tokenFile.GetConstBufferPtr() ), tokenFile.GetDataSize() ) );
            args.push_back( "-H" ); args.push_back( "Authorization: Bearer " + token );
            ASSERT_TRUE( 200 == Curl( args, body ) && Contains( body, "\"hub\":\"testhub\"" ) && Contains( body, "\"nodes\":[" ) );
            args.back() = "Authorization: Bearer wrong-token";
            ASSERT_TRUE( 401 == Curl( args, body ) );

            // a password login: the hashing runs in the pool of the hub, the cookie comes back
            CORE::CProcess::TStringVector login;
            login.push_back( "-D" ); login.push_back( hub.Path( "headers" ) );
            login.push_back( "-c" ); login.push_back( hub.Path( "jar" ) );
            login.push_back( "-H" ); login.push_back( "X-LabLink-Csrf: 1" );
            login.push_back( "--data-urlencode" ); login.push_back( "username=alice" );
            login.push_back( "--data-urlencode" ); login.push_back( "password@" + hub.Path( "password" ) );
            login.push_back( site + "/api/v1/login" );
            ASSERT_TRUE( 200 == Curl( login, body ) && Contains( body, "\"user\":\"alice\"" ) );
            const CORE::CString headers = ReadText( hub.Path( "headers" ) );
            ASSERT_TRUE( Contains( headers, "Set-Cookie: lablink-session=" ) && Contains( headers, "; Path=/; HttpOnly; SameSite=Strict" ) && !Contains( headers, "Secure" ) );
            CORE::CopyFile( hub.Path( "jar" ), hub.Path( "jar-before-logout" ), true );
            CORE::CProcess::TStringVector withCookie;
            withCookie.push_back( "-b" ); withCookie.push_back( hub.Path( "jar" ) );
            withCookie.push_back( site + "/api/v1/session" );
            ASSERT_TRUE( 200 == Curl( withCookie, body ) && Contains( body, "\"role\":\"admin\"" ) );
            login.pop_back();
            login.push_back( site + "/api/v1/login" );
            login[ 2 ] = "-o";
            login[ 3 ] = "/dev/null";
            login[ 9 ] = "password=not-her-password";
            ASSERT_TRUE( 401 == Curl( login, body ) );

            // the Host of a page that rebinds its DNS name is refused
            CORE::CProcess::TStringVector rebound;
            rebound.push_back( "-H" ); rebound.push_back( "Host: evil.example:" + CORE::ToString( webPort ) );
            rebound.push_back( site + "/" );
            ASSERT_TRUE( 403 == Curl( rebound, body ) );

            // logout ends the session on the hub: the cookie of before is worth nothing after it
            CORE::CProcess::TStringVector logout;
            logout.push_back( "-b" ); logout.push_back( hub.Path( "jar" ) );
            logout.push_back( "-X" ); logout.push_back( "POST" );
            logout.push_back( "-H" ); logout.push_back( "X-LabLink-Csrf: 1" );
            logout.push_back( site + "/api/v1/logout" );
            ASSERT_TRUE( 200 == Curl( logout, body ) );
            withCookie[ 1 ] = hub.Path( "jar-before-logout" );
            ASSERT_TRUE( 401 == Curl( withCookie, body ) );
            hub.Stop();
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Web: the hub as a program over HTTPS: the certificate is used, the cookie is Secure with the __Host- prefix" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            GUCEF_TESTFW_SKIP_IF( !IsCurlAvailable(), "curl is not installed" );
            GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl tool is not installed" );
            TESTFW::CTlsTestCerts certs;
            ASSERT_TRUE( certs.IsReady() );
            CHubProgram hub( "gucefLabLink_TestApp_WebHubTls" );
            GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( hub.program ), "gucefLabLink is not next to the test application" );
            CORE::CString output;
            ASSERT_TRUE( 0 == RunWithPassword( hub.program, hub.Path( "state" ), "alice-long-password", "--add-user alice --role viewer", output ) );
            ASSERT_TRUE( WriteText( hub.Path( "password" ), "alice-long-password" ) );

            const UInt16 webPort = TESTFW::CTlsTestServer::FindFreePort();
            CORE::CProcess::TStringVector extra;
            extra.push_back( "--web" );
            extra.push_back( "--web-port" ); extra.push_back( CORE::ToString( webPort ) );
            extra.push_back( "--web-listen" ); extra.push_back( "127.0.0.1" );
            extra.push_back( "--web-cert" ); extra.push_back( certs.GetPath( "good.crt" ) );
            extra.push_back( "--web-key" ); extra.push_back( certs.GetPath( "good.key" ) );
            ASSERT_TRUE( hub.Start( extra ) && WaitForPort( webPort, 30000 ) );
            const CORE::CString site = "https://127.0.0.1:" + CORE::ToString( webPort );
            CORE::CString body;
            CORE::CProcess::TStringVector args;
            args.push_back( "--cacert" ); args.push_back( certs.GetPath( "ca.crt" ) );
            args.push_back( site + "/" );
            ASSERT_TRUE( 200 == Curl( args, body ) && Contains( body, "<title>Lab hub</title>" ) );

            CORE::CProcess::TStringVector login;
            login.push_back( "--cacert" ); login.push_back( certs.GetPath( "ca.crt" ) );
            login.push_back( "-D" ); login.push_back( hub.Path( "headers" ) );
            login.push_back( "-H" ); login.push_back( "X-LabLink-Csrf: 1" );
            login.push_back( "--data-urlencode" ); login.push_back( "username=alice" );
            login.push_back( "--data-urlencode" ); login.push_back( "password@" + hub.Path( "password" ) );
            login.push_back( site + "/api/v1/login" );
            ASSERT_TRUE( 200 == Curl( login, body ) && Contains( body, "\"role\":\"viewer\"" ) && Contains( body, "\"tls\":true" ) );
            const CORE::CString headers = ReadText( hub.Path( "headers" ) );
            ASSERT_TRUE( Contains( headers, "Set-Cookie: __Host-lablink-session=" ) && Contains( headers, "; Path=/; HttpOnly; SameSite=Strict; Secure" ) );

            // the port speaks TLS only
            args.clear();
            args.push_back( "http://127.0.0.1:" + CORE::ToString( webPort ) + "/" );
            ASSERT_TRUE( 200 != Curl( args, body ) );
            hub.Stop();
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
