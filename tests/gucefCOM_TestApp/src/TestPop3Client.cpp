/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_TEST_POP3_SERVER_H
#include "gucef_test_pop3_server.h"
#define GUCEF_TEST_POP3_SERVER_H
#endif /* GUCEF_TEST_POP3_SERVER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COM_CMAILOPERATIONTASK_H
#include "gucefCOM_CMailOperationTask.h"
#define GUCEF_COM_CMAILOPERATIONTASK_H
#endif /* GUCEF_COM_CMAILOPERATIONTASK_H ? */

#include "TestMailShared.h"

#include "TestPop3Client.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace MailTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
RawMessage( const char* subject, const char* body )
{
    return CORE::CString( "From: Alice <alice@example.org>\r\nTo: bob@example.com\r\nSubject: " ) + subject + "\r\nMessage-ID: <" + subject + "@example.org>\r\n\r\n" + body;
}

/*-------------------------------------------------------------------------*/

void
FillMailbox( TESTFW::CTestPop3Server& server )
{
    server.AddMessage( "uid-one", RawMessage( "one", "first body\r\n.starts with a dot\r\n..two dots\r\nlast line\r\n" ) );
    server.AddMessage( "uid-two", RawMessage( "two", "second body\r\n" ) );
    server.AddMessage( "uid-three", RawMessage( "three", "third body\r\n" ) );
}

/*-------------------------------------------------------------------------*/

COM::CPop3ClientSettings
PlainSettings( const TESTFW::CTestPop3Server& server )
{
    COM::CPop3ClientSettings settings;
    MakePlain( settings, server.GetPort() );
    return settings;
}

/*-------------------------------------------------------------------------*/

const COM::CPop3CommandResult*
RunCommand( COM::CPop3Client& client, CPop3Watch& watch, UInt32 requestId )
{
    if ( 0 == requestId )
        return GUCEF_NULL;
    const UInt32 before = watch.resultCount;
    PumpUntilCondition( CounterAtLeast( watch.resultCount, before + 1 ), 8000 );
    return watch.Find( requestId );
}

/*-------------------------------------------------------------------------*/

bool
HasCommand( const GUCEF::vector< CORE::CString >& commands, const char* prefix )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( commands[ i ].StartsWith( prefix ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
AsText( const CORE::CDynamicBuffer& data )
{
    return data.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( data.GetConstBufferPtr() ), data.GetDataSize() ) : CORE::CString();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPop3ClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Pop3Client" );

    CORE::CString whyNoTls;
    const bool tlsPossible = TlsTestingPossible( whyNoTls );
    TESTFW::CTlsTestCerts* certs = tlsPossible ? GUCEF_NEW TESTFW::CTlsTestCerts() : GUCEF_NULL;
    const bool tlsReady = GUCEF_NULL != certs && certs->IsReady();

    GUCEF_TESTFW_TESTCASE( "Pop3Client: USER and PASS, then STAT, LIST, UIDL, RETR with the dots of the message back, TOP, DELE, RSET and QUIT" )
    try
    {
        TESTFW::CTestPop3Server server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CPop3ClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = server.password;
        COM::CPop3Client client;
        CPop3Watch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );
        ASSERT_TRUE( client.HasCapability( "UIDL" ) && client.HasCapability( "top" ) && !client.HasCapability( "STLS" ) );

        const COM::CPop3CommandResult* result = RunCommand( client, watch, client.Stat() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->messageCount && result->totalSize > 100 );
        result = RunCommand( client, watch, client.List() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->entries.size() && 2 == result->entries[ 1 ].number && result->entries[ 1 ].size > 50 );
        result = RunCommand( client, watch, client.Uidl() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->entries.size() && result->entries[ 2 ].uid == "uid-three" );
        result = RunCommand( client, watch, client.Uidl( 2 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 1 == result->entries.size() && result->entries[ 0 ].uid == "uid-two" );
        result = RunCommand( client, watch, client.List( 1 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 1 == result->entries.size() && 1 == result->entries[ 0 ].number );

        result = RunCommand( client, watch, client.Retr( 1 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );
        ASSERT_TRUE( GUCEF_NULL != result && AsText( result->data ) == server.GetMessages()[ 0 ].data );
        result = RunCommand( client, watch, client.Top( 1, 1 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && AsText( result->data ).HasSubstr( "first body" ) >= 0 && AsText( result->data ).HasSubstr( "starts with" ) < 0 );

        // a message that is not there is an error of the command, not of the connection
        result = RunCommand( client, watch, client.Retr( 99 ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->ok && COM::MAILERR_PERMANENT == result->errorClass && client.IsReady() );

        // marked for deletion, and taken back
        result = RunCommand( client, watch, client.Dele( 2 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );
        result = RunCommand( client, watch, client.Retr( 2 ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->ok );
        result = RunCommand( client, watch, client.Rset() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );
        result = RunCommand( client, watch, client.Noop() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );
        result = RunCommand( client, watch, client.Dele( 3 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );

        // commands given together are carried out one after another
        const UInt32 first = client.Stat();
        const UInt32 second = client.Noop();
        ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 15 ), 5000 ) );
        ASSERT_TRUE( GUCEF_NULL != watch.Find( first ) && GUCEF_NULL != watch.Find( second ) && 2 == watch.Find( first )->messageCount );

        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );

        // the server handles the QUIT on a thread of its own: the session is over when the maildrop is updated
        ASSERT_TRUE( server.WaitForCommand( "QUIT", 5000 ) && server.WaitForFinishedConnections( 1, 5000 ) );
        ASSERT_TRUE( 1 == server.GetCommittedDeletes() && 2 == server.GetMessages().size() );
        ASSERT_TRUE( HasCommand( server.GetCommands(), "QUIT" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pop3Client: the QUIT of a client that closes at once reaches a server that is slow, so the deletes are carried out" )
    try
    {
        TESTFW::CTestPop3Server server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CPop3ClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = server.password;
        COM::CPop3Client client;
        CPop3Watch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );

        // the DELE is not answered yet when the client closes: the QUIT goes out right behind it, and the socket is closed with the answer still on its way
        server.replyDelayInMs = 300;
        ASSERT_TRUE( 0 != client.Dele( 2 ) );
        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );

        // the server is still busy with the DELE here, so a look right now sees neither: the test waits for the server
        ASSERT_TRUE( server.WaitForCommand( "QUIT", 5000 ) );
        ASSERT_TRUE( server.WaitForFinishedConnections( 1, 5000 ) );
        ASSERT_TRUE( 1 == server.GetCommittedDeletes() && 2 == server.GetMessages().size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pop3Client: a server without CAPA, APOP, and a login with a wrong password" )
    try
    {
        {
            TESTFW::CTestPop3Server server;
            server.supportCapa = false;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );
            const COM::CPop3CommandResult* result = RunCommand( client, watch, client.Stat() );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->messageCount );
            client.Disconnect();
        }
        {
            TESTFW::CTestPop3Server server;
            server.apopTimestamp = "<1896.697170952@pop.test>";
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.useApop = true;
            settings.userName = "alice";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );
            ASSERT_TRUE( HasCommand( server.GetCommands(), "APOP" ) && !HasCommand( server.GetCommands(), "USER" ) );
            client.Disconnect();
        }
        {
            TESTFW::CTestPop3Server server;
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            settings.password = "wrong";
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_AUTH == watch.error.errorClass && !watch.ready );
        }
        {
            // no login without TLS unless allowed
            TESTFW::CTestPop3Server server;
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
            ASSERT_FALSE( HasCommand( server.GetCommands(), "USER" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pop3Client: authentication with PLAIN, SCRAM-SHA-256 and XOAUTH2, and a token that is refused" )
    try
    {
        const char* mechanisms[] = { "PLAIN", "SCRAM-SHA-256", "XOAUTH2" };
        for ( int m=0; m<3; ++m )
        {
            TESTFW::CTestPop3Server server;
            server.allowUserPass = false;
            server.authMechanisms = mechanisms[ m ];
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            settings.password = 2 == m ? server.token : server.password;
            settings.passwordIsToken = 2 == m;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );
            const COM::CPop3CommandResult* result = RunCommand( client, watch, client.Stat() );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->messageCount );
            client.Disconnect();
        }
        TESTFW::CTestPop3Server server;
        server.allowUserPass = false;
        server.authMechanisms = "XOAUTH2";
        ASSERT_TRUE( server.Start() );
        COM::CPop3ClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = "ya29.refused";
        settings.passwordIsToken = true;
        COM::CPop3Client client;
        CPop3Watch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) );
        ASSERT_TRUE( watch.failed && COM::MAILERR_AUTH == watch.error.errorClass );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pop3Client: STLS upgrades the connection and the capabilities are asked for again; TLS from the start; STLS required and not offered" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !tlsReady, "TLS cannot be tested here" );
        {
            TESTFW::CTestPop3Server server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.offerStls = true;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 15000 ) && watch.ready );
            GUCEF::vector< CORE::CString > commands = server.GetCommands();
            ASSERT_TRUE( commands.size() >= 5 && commands[ 0 ] == "CAPA" && commands[ 1 ] == "STLS" && commands[ 2 ] == "CAPA" );
            const COM::CPop3CommandResult* result = RunCommand( client, watch, client.Retr( 1 ) );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok && AsText( result->data ) == server.GetMessages()[ 0 ].data );
            client.Disconnect();
        }
        {
            TESTFW::CTestPop3Server server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.SetImplicitTls( true );
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_IMPLICIT );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 15000 ) && watch.ready );
            client.Disconnect();
        }
        {
            TESTFW::CTestPop3Server server;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
        }
        {
            TESTFW::CTestPop3Server server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.offerStls = true;
            server.hostile = TESTFW::CTestPop3Server::HOSTILE_INJECT_AFTER_STLS;
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 15000 ) );
            ASSERT_TRUE( watch.failed && !watch.ready );
            ASSERT_FALSE( HasCommand( server.GetCommands(), "USER" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pop3Client: what is new is fetched and parsed, known messages are left alone, and deleted when asked" )
    try
    {
        TESTFW::CTestPop3Server server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CPop3ClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = server.password;

        COM::CPop3FetchOptions options;
        options.knownUids.insert( "uid-one" );
        COM::CPop3FetchResult result;
        ASSERT_TRUE( COM::CPop3Client::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( 3 == result.allUids.size() && 3 == result.messageCountOnServer && 2 == result.messages.size() );
        ASSERT_TRUE( result.messages[ 0 ].uid == "uid-two" && result.messages[ 0 ].parsed && result.messages[ 0 ].message.subject == "two" && result.messages[ 0 ].message.textBody == "second body" );
        ASSERT_TRUE( !result.messages[ 0 ].deleted && 0 == server.GetCommittedDeletes() );

        options.maxMessages = 1;
        ASSERT_TRUE( COM::CPop3Client::FetchNewSync( settings, options, result ) && 1 == result.messages.size() );

        options.maxMessages = 0;
        options.knownUids.clear();
        options.deleteAfterFetch = true;
        ASSERT_TRUE( COM::CPop3Client::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( 3 == result.messages.size() && result.messages[ 0 ].deleted && result.messages[ 2 ].deleted );
        ASSERT_TRUE( result.messages[ 0 ].message.textBody == "first body\n.starts with a dot\n..two dots\nlast line" );
        ASSERT_TRUE( server.WaitForFinishedConnections( 3, 5000 ) );                // the QUIT of the last fetch is carried out by the server after the client is done
        ASSERT_TRUE( 3 == server.GetCommittedDeletes() && 0 == server.GetMessages().size() );

        // nothing there
        ASSERT_TRUE( COM::CPop3Client::FetchNewSync( settings, options, result ) && 0 == result.messages.size() );

        // the asynchronous form does the same on a pool thread
        FillMailbox( server );
        options.deleteAfterFetch = false;
        CORE::CFutureResult future = COM::CMailOperations::FetchNewPop3Async( settings, options );
        ASSERT_TRUE( future.HasAFuture() );
        future.Await( 30000 );
        COM::CPop3FetchResult asyncResult;
        ASSERT_TRUE( COM::CMailOperations::TryGetPop3FetchResult( future, asyncResult ) && asyncResult.success && 3 == asyncResult.messages.size() );

        // a server without UIDL cannot tell us what is new: that is an error, not a guess
        TESTFW::CTestPop3Server noUidl;
        noUidl.supportUidl = false;
        FillMailbox( noUidl );
        ASSERT_TRUE( noUidl.Start() );
        COM::CPop3ClientSettings noUidlSettings = PlainSettings( noUidl );
        noUidlSettings.allowInsecureAuth = true;
        noUidlSettings.userName = "alice";
        noUidlSettings.password = noUidl.password;
        ASSERT_FALSE( COM::CPop3Client::FetchNewSync( noUidlSettings, COM::CPop3FetchOptions(), result ) );
        ASSERT_TRUE( !result.errorDescription.IsNULLOrEmpty() && 0 == result.messages.size() );

        // a server that is not there
        COM::CPop3ClientSettings refused;
        MakePlain( refused, TESTFW::CTlsTestServer::FindFreePort() );
        ASSERT_FALSE( COM::CPop3Client::FetchNewSync( refused, COM::CPop3FetchOptions(), result ) );
        ASSERT_TRUE( COM::MAILERR_NONE != result.errorClass );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    {
        const TESTFW::CTestPop3Server::EHostile modes[] = {
            TESTFW::CTestPop3Server::HOSTILE_BAD_GREETING, TESTFW::CTestPop3Server::HOSTILE_GIANT_STATUS, TESTFW::CTestPop3Server::HOSTILE_ENDLESS_LIST,
            TESTFW::CTestPop3Server::HOSTILE_BAD_UIDL, TESTFW::CTestPop3Server::HOSTILE_ENDLESS_RETR, TESTFW::CTestPop3Server::HOSTILE_DROP_DURING_RETR };
        const char* names[] = { "a greeting that is not POP3", "a status line that does not end", "a capability list that does not end", "a unique id that is not valid", "a message that does not end", "a connection that ends in a message" };
        for ( size_t i=0; i<sizeof( modes ) / sizeof( modes[ 0 ] ); ++i )
        {
            GUCEF_TESTFW_TESTCASE( ( CORE::CString( "Pop3Client: a hostile server cannot make the client read more than the limits: " ) + names[ i ] ).C_String() )
            try
            {
                TESTFW::CTestPop3Server server;
                server.hostile = modes[ i ];
                FillMailbox( server );
                ASSERT_TRUE( server.Start() );
                COM::CPop3ClientSettings settings = PlainSettings( server );
                settings.allowInsecureAuth = true;
                settings.userName = "alice";
                settings.password = server.password;
                settings.maxLineLength = 4096;
                settings.maxResponseCount = 500;
                settings.maxResponseSize = 64 * 1024;
                COM::CPop3Client client;
                CPop3Watch watch;
                watch.Attach( client );
                ASSERT_TRUE( client.Connect( settings ) );
                if ( TESTFW::CTestPop3Server::HOSTILE_BAD_UIDL == modes[ i ] || TESTFW::CTestPop3Server::HOSTILE_ENDLESS_RETR == modes[ i ] || TESTFW::CTestPop3Server::HOSTILE_DROP_DURING_RETR == modes[ i ] )
                {
                    ASSERT_TRUE( PumpUntilTrue( watch.ready, 8000 ) );
                    if ( TESTFW::CTestPop3Server::HOSTILE_BAD_UIDL == modes[ i ] )
                        client.Uidl();
                    else
                        client.Retr( 1 );
                }
                ASSERT_TRUE( PumpUntilTrue( watch.failed, 15000 ) );
                if ( TESTFW::CTestPop3Server::HOSTILE_DROP_DURING_RETR == modes[ i ] )
                    ASSERT_TRUE( COM::MAILERR_CONNECTION == watch.error.errorClass );
                else
                    ASSERT_TRUE( COM::MAILERR_PROTOCOL == watch.error.errorClass );
                // the command that was waiting got its error
                if ( watch.resultCount > 0 )
                    ASSERT_TRUE( !watch.results[ 0 ].ok );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
            GUCEF_TESTFW_TESTCASE_END
        }
    }

    GUCEF_TESTFW_TESTCASE( "Pop3Client: a silent server and slow answers end in a timeout" )
    try
    {
        {
            TESTFW::CTestPop3Server server;
            server.replyDelayInMs = 1500;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.commandTimeoutInMs = 400;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.failed, 8000 ) );
            ASSERT_TRUE( COM::MAILERR_TIMEOUT == watch.error.errorClass );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pop3Client: the password, the user, the token and the content of messages are not in the log" )
    try
    {
        CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
        const Int32 previousLevel = logManager.GetMinLogLevel();
        logManager.SetMinLogLevel( CORE::LOGLEVEL_EVERYTHING );
        TESTFW::CLogTextCapture capture;
        {
            TESTFW::CTestPop3Server server;
            server.password = "UNIQUE-POP3-PASSWORD-4471";
            server.AddMessage( "uid-x", RawMessage( "UNIQUE-POP3-SUBJECT-8812", "UNIQUE-POP3-BODY-3390\r\n" ) );
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice-unique-user";
            server.user = "alice-unique-user";
            settings.password = server.password;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );
            const COM::CPop3CommandResult* result = RunCommand( client, watch, client.Retr( 1 ) );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok );
            client.Disconnect();
        }
        {
            TESTFW::CTestPop3Server server;
            server.allowUserPass = false;
            server.authMechanisms = "XOAUTH2 PLAIN";
            server.token = "ya29.UNIQUE-POP3-TOKEN-6623";
            ASSERT_TRUE( server.Start() );
            COM::CPop3ClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            settings.password = server.token;
            settings.passwordIsToken = true;
            COM::CPop3Client client;
            CPop3Watch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );
            client.Disconnect();
        }
        const CORE::CString text = capture.GetText();
        logManager.SetMinLogLevel( previousLevel );
        ASSERT_TRUE( text.HasSubstr( "POP3 127.0.0.1 C: CAPA" ) >= 0 && text.HasSubstr( "C: USER ***" ) >= 0 && text.HasSubstr( "C: PASS ***" ) >= 0 && text.HasSubstr( "C: AUTH XOAUTH2 ***" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "UNIQUE-POP3-PASSWORD-4471" ) < 0 && text.HasSubstr( "alice-unique-user" ) < 0 && text.HasSubstr( "UNIQUE-POP3-TOKEN-6623" ) < 0 );
        ASSERT_TRUE( text.HasSubstr( "UNIQUE-POP3-SUBJECT-8812" ) < 0 && text.HasSubstr( "UNIQUE-POP3-BODY-3390" ) < 0 );
        const CORE::CString oauthMessage = "user=alice\x01" "auth=Bearer ya29.UNIQUE-POP3-TOKEN-6623\x01\x01";
        ASSERT_TRUE( text.HasSubstr( CORE::Base64Encode( oauthMessage.C_String(), oauthMessage.ByteSize() - 1 ) ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_DELETE certs;
}

/*-------------------------------------------------------------------------*/
