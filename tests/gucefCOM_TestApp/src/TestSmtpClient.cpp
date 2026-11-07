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

#ifndef GUCEF_TEST_SMTP_SERVER_H
#include "gucef_test_smtp_server.h"
#define GUCEF_TEST_SMTP_SERVER_H
#endif /* GUCEF_TEST_SMTP_SERVER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COM_CMAILOPERATIONTASK_H
#include "gucefCOM_CMailOperationTask.h"
#define GUCEF_COM_CMAILOPERATIONTASK_H
#endif /* GUCEF_COM_CMAILOPERATIONTASK_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#include "TestMailShared.h"

#include "TestSmtpClient.h"

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

/** A provider of an OAuth 2.0 token that expired, and is renewed when the server refuses it */
class CRenewingProvider : public COM::CIMailCredentialProvider
{
    public:

    CORE::CString user;
    CORE::CString currentToken;
    CORE::CString renewedToken;
    UInt32 requests;
    UInt32 rejections;
    bool makePending;               /**< the first answer is CRED_PENDING */
    bool wasPending;

    CRenewingProvider( const CORE::CString& userName, const CORE::CString& expired, const CORE::CString& renewed )
        : COM::CIMailCredentialProvider(), user( userName ), currentToken( expired ), renewedToken( renewed ), requests( 0 ), rejections( 0 ), makePending( false ), wasPending( false )
    {
    }

    virtual EResult GetCredential( const COM::CMailCredentialRequest& request, COM::CMailCredential& credential ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++requests;
        if ( makePending )
        {
            wasPending = true;
            makePending = false;
            return CRED_PENDING;
        }
        credential.type = COM::MAILCRED_BEARER_TOKEN;
        credential.userName = user;
        credential.secret = currentToken;
        return CRED_READY;
    }

    virtual void ReportRejected( const COM::CMailCredentialRequest& request ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++rejections;
        currentToken = renewedToken;
    }

    void SignalReady( void )
    {
        CORE::TCloneableUInt32 data( 1 );
        NotifyObservers( CredentialReadyEvent, &data );
    }
};

/*-------------------------------------------------------------------------*/

COM::CSmtpClientSettings
PlainSettings( const TESTFW::CTestSmtpServer& server )
{
    COM::CSmtpClientSettings settings;
    MakePlain( settings, server.GetPort() );
    settings.heloName = "client.test";
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
SendAndWait( COM::CSmtpClient& client, CSmtpWatch& watch, const COM::CMailMessage& message, UInt32& requestId, UInt32 timeoutInMs = 8000 )
{
    const UInt32 before = watch.resultCount;
    CORE::CString error;
    requestId = client.SendMessage( message, &error );
    if ( 0 == requestId )
        return false;
    return PumpUntilCondition( CounterAtLeast( watch.resultCount, before + 1 ), timeoutInMs );
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

UInt32
CountCommands( const GUCEF::vector< CORE::CString >& commands, const char* prefix )
{
    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( commands[ i ].StartsWith( prefix ) )
            ++count;
    return count;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSmtpClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SmtpClient" );

    CORE::CString whyNoTls;
    const bool tlsPossible = TlsTestingPossible( whyNoTls );
    TESTFW::CTlsTestCerts* certs = tlsPossible ? GUCEF_NEW TESTFW::CTlsTestCerts() : GUCEF_NULL;
    const bool tlsReady = GUCEF_NULL != certs && certs->IsReady();

    GUCEF_TESTFW_TESTCASE( "SmtpClient: a message is sent over plain text, the envelope and the data arrive as they were, dots stay" )
    try
    {
        TESTFW::CTestSmtpServer server;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );

        COM::CMailMessage message = MakeMessage( "Plain subject", "first line\n.dot at the start\n..two dots\nlast line" );
        message.cc.push_back( COM::CMailAddress( "carol@example.net" ) );
        message.bcc.push_back( COM::CMailAddress( "secret@example.net" ) );
        UInt32 id = 0;
        ASSERT_TRUE( SendAndWait( client, watch, message, id ) );
        ASSERT_TRUE( watch.ready && !watch.failed );
        const COM::CSmtpSendResult* result = watch.Find( id );
        ASSERT_TRUE( GUCEF_NULL != result && result->success && 250 == result->replyCode && 3 == result->GetAcceptedCount() && 0 == result->GetRejectedCount() );
        ASSERT_TRUE( GUCEF_NULL != result && result->enhancedCode == "2.0.0" && result->replyText.HasSubstr( "queued as 4F2A1" ) >= 0 );

        GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = server.GetMessages();
        ASSERT_TRUE( 1 == received.size() );
        if ( 1 == received.size() )
        {
            ASSERT_TRUE( received[ 0 ].mailFrom == "alice@example.org" && 3 == received[ 0 ].recipients.size() );
            ASSERT_TRUE( received[ 0 ].recipients[ 2 ] == "secret@example.net" && received[ 0 ].mailParameters.HasSubstr( "SIZE=" ) < 0 );
            COM::CMimeParser parser;
            COM::CMailMessage parsed;
            ASSERT_TRUE( parser.ParseMessage( received[ 0 ].data.C_String(), received[ 0 ].data.ByteSize() - 1, parsed ) );
            ASSERT_TRUE( parsed.subject == "Plain subject" && parsed.textBody == "first line\n.dot at the start\n..two dots\nlast line" );
            ASSERT_TRUE( parsed.bcc.empty() && received[ 0 ].data.HasSubstr( "secret@example.net" ) < 0 );
            ASSERT_TRUE( received[ 0 ].data.HasSubstr( "\n.\r\n" ) < 0 );
        }
        GUCEF::vector< CORE::CString > commands = server.GetCommands();
        ASSERT_TRUE( commands.size() >= 5 && commands[ 0 ] == "EHLO client.test" );
        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: several messages go over one connection in order, with and without pipelining" )
    try
    {
        for ( int round=0; round<2; ++round )
        {
            TESTFW::CTestSmtpServer server;
            server.pipelining = 0 == round;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 first = client.SendMessage( MakeMessage( "one", "1" ) );
            UInt32 second = client.SendMessage( MakeMessage( "two", "2" ) );
            UInt32 third = client.SendMessage( MakeMessage( "three", "3" ) );
            ASSERT_TRUE( 0 != first && first != second && second != third );
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 3 ), 8000 ) );
            ASSERT_TRUE( 3 == watch.results.size() && watch.results[ 0 ].requestId == first && watch.results[ 2 ].requestId == third );
            ASSERT_TRUE( watch.results[ 0 ].success && watch.results[ 1 ].success && watch.results[ 2 ].success );
            GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = server.GetMessages();
            ASSERT_TRUE( 3 == received.size() && received[ 1 ].data.HasSubstr( "Subject: two" ) >= 0 );

            // with pipelining MAIL and RCPT are written together: the commands are the same, one connection served all
            ASSERT_TRUE( 1 == server.GetAcceptedConnections() );
            ASSERT_TRUE( 3 == CountCommands( server.GetCommands(), "MAIL FROM:" ) && 3 == CountCommands( server.GetCommands(), "DATA" ) );
            client.Disconnect();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: the QUIT of a client that closes at once reaches a server that is slow" )
    try
    {
        TESTFW::CTestSmtpServer server;
        server.pipelining = false;                      // one command at a time: exactly one answer is outstanding when the client closes
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilTrue( watch.settled, 8000 ) && watch.ready );

        // the MAIL FROM is not answered yet when the client closes: the QUIT goes out right behind it, and the socket is closed with the answer still on its way
        server.replyDelayInMs = 300;
        ASSERT_TRUE( 0 != client.SendMessage( MakeMessage( "slow", "text" ) ) );
        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );

        // the server is still busy with the MAIL FROM here, so a look right now sees no QUIT: the test waits for the server
        ASSERT_TRUE( server.WaitForCommand( "MAIL FROM:", 5000 ) );
        ASSERT_TRUE( server.WaitForCommand( "QUIT", 5000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: HELO is the fallback when the server does not know EHLO, and then there are no extensions" )
    try
    {
        TESTFW::CTestSmtpServer server;
        server.supportEhlo = false;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        UInt32 id = 0;
        ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "via helo", "text" ), id ) );
        ASSERT_TRUE( watch.Find( id )->success && !client.HasExtension( "PIPELINING" ) );
        ASSERT_TRUE( HasCommand( server.GetCommands(), "HELO client.test" ) );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: authentication with PLAIN, LOGIN, SCRAM-SHA-256 and XOAUTH2, and what happens with a wrong password" )
    try
    {
        const char* mechanisms[] = { "PLAIN", "LOGIN", "SCRAM-SHA-256", "XOAUTH2" };
        for ( int m=0; m<4; ++m )
        {
            TESTFW::CTestSmtpServer server;
            server.authMechanisms = mechanisms[ m ];
            server.requireAuth = true;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            settings.password = 3 == m ? server.token : server.password;
            settings.passwordIsToken = 3 == m;
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = 0;
            ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "authenticated", "text" ), id ) );
            ASSERT_TRUE( watch.Find( id )->success && 1 == server.GetAuthenticatedSessions() && 1 == server.GetMessages().size() );
            client.Disconnect();
        }

        // the wrong password: one attempt, the failure is an authentication failure, the message is not sent and told so
        TESTFW::CTestSmtpServer server;
        server.authMechanisms = "PLAIN LOGIN";
        server.requireAuth = true;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = "wrong";
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        UInt32 id = client.SendMessage( MakeMessage( "never", "text" ) );
        ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 8000 ) );
        ASSERT_TRUE( watch.failed && COM::MAILERR_AUTH == watch.error.errorClass && 535 == watch.error.serverCode );
        ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 1 ), 2000 ) );
        ASSERT_TRUE( !watch.results[ 0 ].success && COM::MAILERR_AUTH == watch.results[ 0 ].errorClass && 0 == server.GetMessages().size() );
        ASSERT_TRUE( 1 == server.GetAuthAttempts() && COM::CMailClientBase::CLIENTSTATE_DISCONNECTED == client.GetState() );
        (void) id;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: credentials are not sent without TLS unless the settings allow it, and the server is told nothing" )
    try
    {
        TESTFW::CTestSmtpServer server;
        server.authMechanisms = "PLAIN";
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        settings.userName = "alice";
        settings.password = server.password;
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 8000 ) );
        ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
        ASSERT_TRUE( 0 == server.GetAuthAttempts() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: the access token comes from a provider, an expired one is renewed once, and a provider that needs time is waited for" )
    try
    {
        for ( int round=0; round<2; ++round )
        {
            TESTFW::CTestSmtpServer server;
            server.authMechanisms = "XOAUTH2";
            server.requireAuth = true;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            CRenewingProvider provider( "alice", 0 == round ? "ya29.expired" : server.token, server.token );
            provider.makePending = 1 == round;
            COM::CSmtpClient client;
            client.SetCredentialProvider( &provider );
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = client.SendMessage( MakeMessage( "token", "text" ) );
            ASSERT_TRUE( 0 != id );
            if ( 1 == round )
            {
                PumpFor( 300 );
                ASSERT_TRUE( provider.wasPending && !watch.ready );
                provider.SignalReady();
            }
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 1 ), 8000 ) );
            ASSERT_TRUE( watch.results[ 0 ].success );
            if ( 0 == round )
            {
                ASSERT_TRUE( 1 == provider.rejections && 2 == server.GetAuthAttempts() && 1 == server.GetAuthenticatedSessions() );
            }
            else
            {
                ASSERT_TRUE( 0 == provider.rejections && 1 == server.GetAuthAttempts() );
            }
            client.Disconnect();
        }

        // a token that stays refused fails the connection after one more try
        TESTFW::CTestSmtpServer server;
        server.authMechanisms = "XOAUTH2";
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        CRenewingProvider provider( "alice", "ya29.bad", "ya29.still-bad" );
        COM::CSmtpClient client;
        client.SetCredentialProvider( &provider );
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 8000 ) );
        ASSERT_TRUE( watch.failed && COM::MAILERR_AUTH == watch.error.errorClass && 2 == server.GetAuthAttempts() && provider.rejections >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: the result tells which recipients were taken, a temporary failure from a permanent one, and failIfAnyRecipientRejected" )
    try
    {
        TESTFW::CTestSmtpServer server;
        server.rejectRecipients.insert( "gone@example.net" );
        server.tempRejectRecipients.insert( "busy@example.net" );
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );

        COM::CMailMessage message = MakeMessage( "partial", "text" );
        message.cc.push_back( COM::CMailAddress( "gone@example.net" ) );
        message.cc.push_back( COM::CMailAddress( "busy@example.net" ) );
        UInt32 id = 0;
        ASSERT_TRUE( SendAndWait( client, watch, message, id ) );
        const COM::CSmtpSendResult* result = watch.Find( id );
        ASSERT_TRUE( GUCEF_NULL != result && result->success && 1 == result->GetAcceptedCount() && 2 == result->GetRejectedCount() );
        ASSERT_TRUE( GUCEF_NULL != result && 3 == result->recipients.size() );
        if ( GUCEF_NULL != result && 3 == result->recipients.size() )
        {
            ASSERT_TRUE( result->recipients[ 1 ].address == "gone@example.net" && COM::MAILERR_PERMANENT == result->recipients[ 1 ].errorClass && 550 == result->recipients[ 1 ].replyCode && result->recipients[ 1 ].enhancedCode == "5.1.1" );
            ASSERT_TRUE( COM::MAILERR_TEMPORARY == result->recipients[ 2 ].errorClass && 450 == result->recipients[ 2 ].replyCode );
        }
        ASSERT_TRUE( 1 == server.GetMessages().size() && 1 == server.GetMessages()[ 0 ].recipients.size() );

        // all of them refused: reset, not sent
        COM::CMailMessage nobody = MakeMessage( "nobody", "text" );
        nobody.to.clear();
        nobody.to.push_back( COM::CMailAddress( "gone@example.net" ) );
        ASSERT_TRUE( SendAndWait( client, watch, nobody, id ) );
        result = watch.Find( id );
        ASSERT_TRUE( GUCEF_NULL != result && !result->success && COM::MAILERR_PERMANENT == result->errorClass && 0 == result->GetAcceptedCount() );
        ASSERT_TRUE( HasCommand( server.GetCommands(), "RSET" ) && 1 == server.GetMessages().size() );

        // the connection is still good for the next message
        ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "after", "text" ), id ) );
        ASSERT_TRUE( watch.Find( id )->success && 2 == server.GetMessages().size() );
        client.Disconnect();

        // strict
        COM::CSmtpClientSettings strict = PlainSettings( server );
        strict.failIfAnyRecipientRejected = true;
        COM::CSmtpClient strictClient;
        CSmtpWatch strictWatch;
        strictWatch.Attach( strictClient );
        ASSERT_TRUE( strictClient.Connect( strict ) );
        ASSERT_TRUE( SendAndWait( strictClient, strictWatch, message, id ) );
        ASSERT_TRUE( !strictWatch.Find( id )->success && 2 == server.GetMessages().size() );
        strictClient.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: a sender that is refused, a message that is refused after the data, and a message that is too large for the server" )
    try
    {
        {
            TESTFW::CTestSmtpServer server;
            server.rejectSender = true;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = 0;
            ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "s", "t" ), id ) );
            ASSERT_TRUE( !watch.Find( id )->success && COM::MAILERR_PERMANENT == watch.Find( id )->errorClass && 550 == watch.Find( id )->replyCode );
            client.Disconnect();
        }
        {
            TESTFW::CTestSmtpServer server;
            server.dataReplyCode = 451;
            server.dataReplyText = "4.3.0 Try again later";
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = 0;
            ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "s", "t" ), id ) );
            ASSERT_TRUE( !watch.Find( id )->success && COM::MAILERR_TEMPORARY == watch.Find( id )->errorClass && 451 == watch.Find( id )->replyCode );
            client.Disconnect();
        }
        {
            TESTFW::CTestSmtpServer server;
            server.sizeLimit = 2000;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = 0;
            COM::CMailMessage large = MakeMessage( "large", "text" );
            GUCEF::vector< char > filler( 10000, 'x' );
            large.attachments.push_back( COM::CMailAttachment( "big.bin", "application/octet-stream", &filler[ 0 ], 10000 ) );
            ASSERT_TRUE( SendAndWait( client, watch, large, id ) );
            ASSERT_TRUE( !watch.Find( id )->success && COM::MAILERR_LOCAL == watch.Find( id )->errorClass && 2000 == client.GetServerMaxSize() );
            ASSERT_TRUE( 0 == CountCommands( server.GetCommands(), "MAIL FROM:" ) );
            ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "small", "t" ), id ) && watch.Find( id )->success );
            ASSERT_TRUE( HasCommand( server.GetCommands(), "MAIL FROM:<alice@example.org> SIZE=" ) );
            client.Disconnect();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: international addresses need SMTPUTF8, and 8 bit data needs 8BITMIME; raw data is sent as given with its line ends repaired" )
    try
    {
        TESTFW::CTestSmtpServer server;
        server.smtpUtf8 = false;
        server.eightBitMime = false;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );

        COM::CMailMessage international = MakeMessage( "intl", "text" );
        international.to[ 0 ] = COM::CMailAddress( "j\xC3\xBC" "rgen@example.com" );
        UInt32 id = 0;
        ASSERT_TRUE( SendAndWait( client, watch, international, id ) );
        ASSERT_TRUE( !watch.Find( id )->success && COM::MAILERR_LOCAL == watch.Find( id )->errorClass );

        COM::CMailAddress::TMailAddressVector recipients;
        recipients.push_back( COM::CMailAddress( "bob@example.com" ) );
        const char* eightBit = "Subject: x\r\n\r\ncaf\xC3\xA9\r\n";
        id = client.SendRaw( COM::CMailAddress( "alice@example.org" ), recipients, eightBit, (UInt32) strlen( eightBit ) );
        ASSERT_TRUE( 0 != id && PumpUntilCondition( CounterAtLeast( watch.resultCount, 2 ), 5000 ) );
        ASSERT_TRUE( !watch.Find( id )->success && COM::MAILERR_LOCAL == watch.Find( id )->errorClass );

        // a message with a NUL is refused at once, and so is a sender that is no address
        const char withNul[] = "Subject: x\r\n\r\na\0b\r\n";
        CORE::CString reason;
        ASSERT_TRUE( 0 == client.SendRaw( COM::CMailAddress( "alice@example.org" ), recipients, withNul, sizeof( withNul ), &reason ) && !reason.IsNULLOrEmpty() );
        client.Disconnect();

        // the other way: both offered
        TESTFW::CTestSmtpServer capable;
        capable.smtpUtf8 = true;
        capable.eightBitMime = true;
        ASSERT_TRUE( capable.Start() );
        COM::CSmtpClientSettings capableSettings = PlainSettings( capable );
        COM::CSmtpClient capableClient;
        CSmtpWatch capableWatch;
        capableWatch.Attach( capableClient );
        ASSERT_TRUE( capableClient.Connect( capableSettings ) );
        ASSERT_TRUE( SendAndWait( capableClient, capableWatch, international, id ) && capableWatch.Find( id )->success );
        bool utf8Requested = false;
        GUCEF::vector< CORE::CString > capableCommands = capable.GetCommands();
        for ( size_t i=0; i<capableCommands.size(); ++i )
            utf8Requested = utf8Requested || ( capableCommands[ i ].StartsWith( "MAIL FROM:" ) && capableCommands[ i ].HasSubstr( " SMTPUTF8" ) >= 0 );
        ASSERT_TRUE( utf8Requested );
        id = capableClient.SendRaw( COM::CMailAddress( "alice@example.org" ), recipients, "Subject: x\n\nbare\nline\rends\r\n.\r\ncaf\xC3\xA9\n", 36 );
        ASSERT_TRUE( 0 != id && PumpUntilCondition( CounterAtLeast( capableWatch.resultCount, 2 ), 5000 ) );
        ASSERT_TRUE( capableWatch.Find( id )->success );
        GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = capable.GetMessages();
        ASSERT_TRUE( 2 == received.size() && received[ 1 ].mailParameters.HasSubstr( "8BITMIME" ) >= 0 );
        // bare line ends are CR LF, the lone dot line was stuffed so it is data and not the end
        ASSERT_TRUE( received[ 1 ].data == "Subject: x\r\n\r\nbare\r\nline\r\nends\r\n.\r\ncaf\xC3\xA9\r\n" );
        capableClient.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: addresses and subjects with line breaks do not become commands or headers on the wire" )
    try
    {
        TESTFW::CTestSmtpServer server;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );

        // an address with a line break is refused before it is queued
        COM::CMailMessage evil = MakeMessage( "evil", "text" );
        COM::CMailAddress sneaky;
        sneaky.localPart = "x";
        sneaky.domain = "example.com>\r\nRCPT TO:<victim@example.org";
        evil.to.push_back( sneaky );
        CORE::CString reason;
        ASSERT_TRUE( 0 == client.SendMessage( evil, &reason ) && !reason.IsNULLOrEmpty() );
        COM::CMailAddress::TMailAddressVector recipients;
        recipients.push_back( sneaky );
        ASSERT_TRUE( 0 == client.SendRaw( COM::CMailAddress( "alice@example.org" ), recipients, "x", 1 ) );
        COM::CMailAddress badSender;
        badSender.localPart = "a\r\nMAIL FROM:<b";
        badSender.domain = "example.org";
        recipients.clear();
        recipients.push_back( COM::CMailAddress( "bob@example.com" ) );
        ASSERT_TRUE( 0 == client.SendRaw( badSender, recipients, "x", 1 ) );

        // a subject and a name with line breaks are encoded
        COM::CMailMessage subject = MakeMessage( "Hi\r\nBcc: evil@example.net\r\n\r\ninjected body", "real body" );
        subject.from[ 0 ].displayName = "Eve\r\nBcc: evil2@example.net";
        UInt32 id = 0;
        ASSERT_TRUE( SendAndWait( client, watch, subject, id ) && watch.Find( id )->success );
        GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = server.GetMessages();
        ASSERT_TRUE( 1 == received.size() );
        if ( 1 == received.size() )
        {
            COM::CMimeParser parser;
            COM::CMailMessage parsed;
            ASSERT_TRUE( parser.ParseMessage( received[ 0 ].data.C_String(), received[ 0 ].data.ByteSize() - 1, parsed ) );
            ASSERT_FALSE( parsed.headers.Has( "Bcc" ) );
            ASSERT_TRUE( parsed.textBody == "real body" && received[ 0 ].data.HasSubstr( "evil" ) < 0 );
        }
        ASSERT_TRUE( 1 == CountCommands( server.GetCommands(), "RCPT TO:" ) && 1 == CountCommands( server.GetCommands(), "MAIL FROM:" ) );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: STARTTLS upgrades the connection, asks for the extensions again, and then the login goes through" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !tlsReady, "TLS cannot be tested here" );
        TESTFW::CTestSmtpServer server;
        server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
        server.offerStartTls = true;
        server.authOnlyAfterTls = true;
        server.authMechanisms = "PLAIN";
        server.requireAuth = true;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings;
        MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
        settings.userName = "alice";
        settings.password = server.password;
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        UInt32 id = 0;
        ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "secure", "text" ), id, 15000 ) );
        ASSERT_TRUE( watch.Find( id )->success && 1 == server.GetAuthenticatedSessions() );
        GUCEF::vector< CORE::CString > commands = server.GetCommands();
        ASSERT_TRUE( 2 == CountCommands( commands, "EHLO" ) && HasCommand( commands, "STARTTLS" ) );
        ASSERT_TRUE( commands.size() > 3 && commands[ 1 ] == "STARTTLS" && commands[ 2 ].StartsWith( "EHLO" ) );
        ASSERT_TRUE( client.GetState() == COM::CMailClientBase::CLIENTSTATE_READY );
        client.Disconnect();

        // a server that does not offer what is required: no downgrade
        TESTFW::CTestSmtpServer plainServer;
        ASSERT_TRUE( plainServer.Start() );
        COM::CSmtpClientSettings required;
        MakeSecure( required, plainServer.GetPort(), *certs, COM::MAILTLS_STARTTLS );
        COM::CSmtpClient requiredClient;
        CSmtpWatch requiredWatch;
        requiredWatch.Attach( requiredClient );
        ASSERT_TRUE( requiredClient.Connect( required ) );
        ASSERT_TRUE( PumpUntilCondition( Settled( requiredWatch ), 8000 ) );
        ASSERT_TRUE( requiredWatch.failed && COM::MAILERR_TLS == requiredWatch.error.errorClass && !requiredWatch.ready );
        ASSERT_TRUE( 0 == plainServer.GetMessages().size() );

        // if offered: plain text is taken when there is no STARTTLS, but a login is not sent
        TESTFW::CTestSmtpServer optional;
        optional.authMechanisms = "PLAIN";
        ASSERT_TRUE( optional.Start() );
        COM::CSmtpClientSettings optionalSettings;
        MakeSecure( optionalSettings, optional.GetPort(), *certs, COM::MAILTLS_STARTTLS_IF_OFFERED );
        COM::CSmtpClient optionalClient;
        CSmtpWatch optionalWatch;
        optionalWatch.Attach( optionalClient );
        ASSERT_TRUE( optionalClient.Connect( optionalSettings ) );
        UInt32 optionalId = 0;
        ASSERT_TRUE( SendAndWait( optionalClient, optionalWatch, MakeMessage( "optional", "text" ), optionalId ) && optionalWatch.Find( optionalId )->success );
        optionalClient.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: TLS from the first byte, a certificate nobody trusts and a certificate for another name are refused" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !tlsReady, "TLS cannot be tested here" );
        {
            TESTFW::CTestSmtpServer server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.SetImplicitTls( true );
            server.authMechanisms = "PLAIN SCRAM-SHA-256";
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_IMPLICIT );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = 0;
            ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "implicit", "text" ), id, 15000 ) );
            ASSERT_TRUE( watch.Find( id )->success && 1 == server.GetAuthenticatedSessions() );
            // SCRAM is preferred over PLAIN
            ASSERT_TRUE( HasCommand( server.GetCommands(), "AUTH SCRAM-SHA-256" ) && !HasCommand( server.GetCommands(), "AUTH PLAIN" ) );
            ASSERT_TRUE( 1 == CountCommands( server.GetCommands(), "EHLO" ) );
            client.Disconnect();
        }
        {
            TESTFW::CTestSmtpServer server;
            server.SetTlsFiles( certs->GetPath( "otherca_leaf.crt" ), certs->GetPath( "otherca_leaf.key" ) );
            server.SetImplicitTls( true );
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_IMPLICIT );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 15000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass && !watch.ready );
        }
        {
            TESTFW::CTestSmtpServer server;
            server.SetTlsFiles( certs->GetPath( "wrongname.crt" ), certs->GetPath( "wrongname.key" ) );
            server.SetImplicitTls( true );
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_IMPLICIT );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 15000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: text that the server sends in plain text after it agreed to STARTTLS is not believed" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !tlsReady, "TLS cannot be tested here" );
        TESTFW::CTestSmtpServer server;
        server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
        server.offerStartTls = true;
        server.authMechanisms = "PLAIN";
        server.hostile = TESTFW::CTestSmtpServer::HOSTILE_INJECT_AFTER_STARTTLS;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings;
        MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
        settings.userName = "alice";
        settings.password = server.password;
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        UInt32 id = client.SendMessage( MakeMessage( "inject", "text" ) );
        ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 15000 ) );
        ASSERT_TRUE( watch.failed && !watch.ready );
        ASSERT_TRUE( COM::MAILERR_PROTOCOL == watch.error.errorClass || COM::MAILERR_TLS == watch.error.errorClass );
        ASSERT_TRUE( 0 == server.GetAuthAttempts() && 0 == server.GetMessages().size() );
        (void) id;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: a server that drops the connection, or is silent, ends the connection and the waiting messages with an error" )
    try
    {
        {
            TESTFW::CTestSmtpServer server;
            server.dropAfterCommands = 1;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            client.SendMessage( MakeMessage( "a", "1" ) );
            client.SendMessage( MakeMessage( "b", "2" ) );
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 2 ), 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_CONNECTION == watch.error.errorClass );
            ASSERT_TRUE( !watch.results[ 0 ].success && !watch.results[ 1 ].success && COM::MAILERR_CONNECTION == watch.results[ 0 ].errorClass );
            ASSERT_TRUE( 0 == client.GetPendingCount() );
        }
        {
            // no greeting at all
            TESTFW::CTestSmtpServer server;
            server.hostile = TESTFW::CTestSmtpServer::HOSTILE_NO_GREETING;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            settings.connectTimeoutInMs = 600;
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 5000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TIMEOUT == watch.error.errorClass );
        }
        {
            // answers that come later than the client waits
            TESTFW::CTestSmtpServer server;
            server.replyDelayInMs = 1500;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            settings.commandTimeoutInMs = 400;
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TIMEOUT == watch.error.errorClass );
        }
        {
            // the end of the data takes long
            TESTFW::CTestSmtpServer server;
            server.dataDelayInMs = 1500;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            settings.dataTimeoutInMs = 400;
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            client.SendMessage( MakeMessage( "slow", "1" ) );
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 1 ), 8000 ) );
            ASSERT_TRUE( !watch.results[ 0 ].success && COM::MAILERR_TIMEOUT == watch.results[ 0 ].errorClass );
        }
        {
            // a connection that is refused
            COM::CSmtpClientSettings settings;
            MakePlain( settings, TESTFW::CTlsTestServer::FindFreePort() );
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_CONNECTION == watch.error.errorClass );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    {
        const TESTFW::CTestSmtpServer::EHostile modes[] = {
            TESTFW::CTestSmtpServer::HOSTILE_GIANT_GREETING, TESTFW::CTestSmtpServer::HOSTILE_ENDLESS_EHLO, TESTFW::CTestSmtpServer::HOSTILE_BAD_GREETING,
            TESTFW::CTestSmtpServer::HOSTILE_MIXED_CODES, TESTFW::CTestSmtpServer::HOSTILE_UNSOLICITED_REPLY, TESTFW::CTestSmtpServer::HOSTILE_DROP_AFTER_EHLO };
        const char* names[] = { "a greeting line that does not end", "an EHLO reply that does not end", "a greeting that is not SMTP", "a multi line reply with different codes", "a reply that nobody asked for", "a connection that is closed after EHLO" };
        for ( size_t i=0; i<sizeof( modes ) / sizeof( modes[ 0 ] ); ++i )
        {
            GUCEF_TESTFW_TESTCASE( ( CORE::CString( "SmtpClient: a hostile server cannot make the client believe or read more than it should: " ) + names[ i ] ).C_String() )
            try
            {
                TESTFW::CTestSmtpServer server;
                server.hostile = modes[ i ];
                ASSERT_TRUE( server.Start() );
                COM::CSmtpClientSettings settings = PlainSettings( server );
                settings.maxLineLength = 4096;
                COM::CSmtpClient client;
                CSmtpWatch watch;
                watch.Attach( client );
                ASSERT_TRUE( client.Connect( settings ) );
                client.SendMessage( MakeMessage( "h", "t" ) );
                if ( TESTFW::CTestSmtpServer::HOSTILE_UNSOLICITED_REPLY == modes[ i ] )
                {
                    // the extra reply of the server is taken as the answer to a command and the answers are out of step from then on:
                    // the connection may go on, the message must not be reported as sent
                    ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.resultCount, 1 ), 10000 ) );
                    ASSERT_TRUE( !watch.results[ 0 ].success );
                    ASSERT_TRUE( 0 == server.GetMessages().size() );
                    continue;
                }
                ASSERT_TRUE( PumpUntilTrue( watch.failed, 10000 ) );
                ASSERT_TRUE( 0 == server.GetMessages().size() );
                if ( TESTFW::CTestSmtpServer::HOSTILE_DROP_AFTER_EHLO != modes[ i ] )
                    ASSERT_TRUE( COM::MAILERR_PROTOCOL == watch.error.errorClass );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
            GUCEF_TESTFW_TESTCASE_END
        }
    }

    GUCEF_TESTFW_TESTCASE( "SmtpClient: the password, the token, the encoded login and the content of messages are not in the log" )
    try
    {
        // the trace of the protocol is at the lowest level of the log: it has to be asked for to look at it
        CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
        const Int32 previousLevel = logManager.GetMinLogLevel();
        logManager.SetMinLogLevel( CORE::LOGLEVEL_EVERYTHING );
        TESTFW::CLogTextCapture capture;
        for ( int round=0; round<3; ++round )
        {
            TESTFW::CTestSmtpServer server;
            server.authMechanisms = 0 == round ? "PLAIN" : ( 1 == round ? "LOGIN" : "XOAUTH2" );
            server.requireAuth = true;
            ASSERT_TRUE( server.Start() );
            COM::CSmtpClientSettings settings = PlainSettings( server );
            settings.allowInsecureAuth = true;
            settings.userName = "alice";
            settings.password = 2 == round ? server.token : server.password;
            settings.passwordIsToken = 2 == round;
            COM::CSmtpClient client;
            CSmtpWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) );
            UInt32 id = 0;
            ASSERT_TRUE( SendAndWait( client, watch, MakeMessage( "UNIQUE-SUBJECT-7731", "UNIQUE-BODY-5521 line" ), id ) && watch.Find( id )->success );
            client.Disconnect();
        }
        // and a failed login
        TESTFW::CTestSmtpServer server;
        server.authMechanisms = "PLAIN";
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = "WRONG-PASSWORD-9917";
        COM::CSmtpClient client;
        CSmtpWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( client.Connect( settings ) );
        ASSERT_TRUE( PumpUntilCondition( Settled( watch ), 8000 ) );

        const CORE::CString text = capture.GetText();
        ASSERT_TRUE( text.HasSubstr( "EHLO" ) >= 0 && text.HasSubstr( "AUTH PLAIN" ) >= 0 );     // the trace is there
        ASSERT_TRUE( text.HasSubstr( "s3cret-p@ss" ) < 0 && text.HasSubstr( "ya29.token-of-alice" ) < 0 && text.HasSubstr( "WRONG-PASSWORD-9917" ) < 0 );
        const CORE::CString plainLogin = CORE::Base64Encode( "\0alice\0s3cret-p@ss", 18 );
        ASSERT_TRUE( text.HasSubstr( plainLogin ) < 0 && text.HasSubstr( CORE::Base64Encode( "s3cret-p@ss", 11 ) ) < 0 && text.HasSubstr( CORE::Base64Encode( "alice", 5 ) ) < 0 );
        ASSERT_TRUE( text.HasSubstr( CORE::Base64Encode( "user=alice\x01" "auth=Bearer ya29.token-of-alice\x01\x01", 44 ) ) < 0 );
        ASSERT_TRUE( text.HasSubstr( "UNIQUE-SUBJECT-7731" ) < 0 && text.HasSubstr( "UNIQUE-BODY-5521" ) < 0 );
        ASSERT_TRUE( text.HasSubstr( "alice@example.org" ) < 0 && text.HasSubstr( "bob@example.com" ) < 0 );
        ASSERT_TRUE( text.HasSubstr( "AUTH PLAIN ***" ) >= 0 && text.HasSubstr( "AUTH LOGIN ***" ) >= 0 && text.HasSubstr( "AUTH XOAUTH2 ***" ) >= 0 );
        logManager.SetMinLogLevel( previousLevel );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SmtpClient: a message is sent from the calling thread with the blocking function, and from a pool thread with the future" )
    try
    {
        TESTFW::CTestSmtpServer server;
        server.authMechanisms = "PLAIN";
        server.requireAuth = true;
        ASSERT_TRUE( server.Start() );
        COM::CSmtpClientSettings settings = PlainSettings( server );
        settings.allowInsecureAuth = true;
        settings.userName = "alice";
        settings.password = server.password;

        COM::CSmtpSendResult result;
        ASSERT_TRUE( COM::CSmtpClient::SendMessageSync( settings, MakeMessage( "sync", "text" ), result ) );
        ASSERT_TRUE( result.success && 1 == result.GetAcceptedCount() && 1 == server.GetMessages().size() );
        ASSERT_TRUE( server.WaitForCommand( "QUIT", 5000 ) );                       // the client closes right after it, the server reads it on a thread of its own
        ASSERT_TRUE( HasCommand( server.GetCommands(), "QUIT" ) );

        CORE::CFutureResult future = COM::CMailOperations::SendMessageAsync( settings, MakeMessage( "async", "text" ) );
        ASSERT_TRUE( future.HasAFuture() );
        future.Await( 20000 );
        COM::CSmtpSendResult asyncResult;
        ASSERT_TRUE( COM::CMailOperations::TryGetSendResult( future, asyncResult ) && asyncResult.success );
        ASSERT_TRUE( 2 == server.GetMessages().size() && server.GetMessages()[ 1 ].data.HasSubstr( "Subject: async" ) >= 0 );

        // failures are results too
        COM::CSmtpClientSettings refused;
        MakePlain( refused, TESTFW::CTlsTestServer::FindFreePort() );
        COM::CSmtpSendResult failed;
        ASSERT_FALSE( COM::CSmtpClient::SendMessageSync( refused, MakeMessage( "x", "y" ), failed ) );
        ASSERT_TRUE( !failed.success && COM::MAILERR_NONE != failed.errorClass );
        COM::CMailMessage invalid;
        ASSERT_FALSE( COM::CSmtpClient::SendMessageSync( settings, invalid, failed ) );
        ASSERT_TRUE( COM::MAILERR_LOCAL == failed.errorClass );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_DELETE certs;
}

/*-------------------------------------------------------------------------*/
