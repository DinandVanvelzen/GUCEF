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

#ifndef GUCEF_TEST_IMAP_SERVER_H
#include "gucef_test_imap_server.h"
#define GUCEF_TEST_IMAP_SERVER_H
#endif /* GUCEF_TEST_IMAP_SERVER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COM_CMAILOPERATIONTASK_H
#include "gucefCOM_CMailOperationTask.h"
#define GUCEF_COM_CMAILOPERATIONTASK_H
#endif /* GUCEF_COM_CMAILOPERATIONTASK_H ? */

#include "TestMailShared.h"

#include "TestImapClient.h"

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
RawMessage( const char* subject, const char* body, const char* extraHeader = "" )
{
    return CORE::CString( "From: Alice <alice@example.org>\r\nTo: bob@example.com\r\nSubject: " ) + subject + "\r\nMessage-ID: <" + subject + "@example.org>\r\n" + extraHeader + "\r\n" + body;
}

/*-------------------------------------------------------------------------*/

void
FillMailbox( TESTFW::CTestImapServer& server )
{
    server.AddMessage( RawMessage( "one", "first body\r\nline two\r\n", "X-Redux-Game: g-1234\r\n" ) );
    server.AddMessage( RawMessage( "two", "second body\r\n", "X-Redux-Game: g-9999\r\n" ) );
    GUCEF::vector< CORE::CString > seen;
    seen.push_back( "\\Seen" );
    server.AddMessage( RawMessage( "three", "third body\r\n" ), seen );
}

/*-------------------------------------------------------------------------*/

COM::CImapClientSettings
PlainSettings( const TESTFW::CTestImapServer& server )
{
    COM::CImapClientSettings settings;
    MakePlain( settings, server.GetPort() );
    settings.maxLiteralSize = 1024 * 1024;
    settings.maxResponseSize = 2 * 1024 * 1024;
    return settings;
}

/*-------------------------------------------------------------------------*/

COM::CImapClientSettings
LoginSettings( const TESTFW::CTestImapServer& server )
{
    COM::CImapClientSettings settings = PlainSettings( server );
    settings.allowInsecureAuth = true;
    settings.userName = "alice";
    settings.password = server.password;
    return settings;
}

/*-------------------------------------------------------------------------*/

const COM::CImapCommandResult*
Wait( CImapWatch& watch, UInt32 requestId, UInt32 timeoutInMs = 8000 )
{
    if ( 0 == requestId )
        return GUCEF_NULL;
    for ( UInt32 waited=0; waited<timeoutInMs && GUCEF_NULL == watch.Find( requestId ); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return watch.Find( requestId );
}

/*-------------------------------------------------------------------------*/

bool
Connected( COM::CImapClient& client, CImapWatch& watch, const COM::CImapClientSettings& settings )
{
    return client.Connect( settings ) && PumpUntilTrue( watch.settled, 15000 ) && watch.ready;
}

/*-------------------------------------------------------------------------*/

bool
HasCommand( const GUCEF::vector< CORE::CString >& commands, const char* prefix )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( commands[ i ].HasSubstr( prefix ) >= 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CountCommands( const GUCEF::vector< CORE::CString >& commands, const char* text )
{
    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( commands[ i ].HasSubstr( text ) >= 0 )
            ++count;
    return count;
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
PerformImapClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ImapClient" );

    CORE::CString whyNoTls;
    const bool tlsPossible = TlsTestingPossible( whyNoTls );
    TESTFW::CTlsTestCerts* certs = tlsPossible ? GUCEF_NEW TESTFW::CTlsTestCerts() : GUCEF_NULL;
    const bool tlsReady = GUCEF_NULL != certs && certs->IsReady();

    GUCEF_TESTFW_TESTCASE( "ImapClient: LOGIN, CAPABILITY, LIST, SELECT with what the server says of the mailbox, STATUS and the end of the session" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( client.HasCapability( "IDLE" ) && client.HasCapability( "uidplus" ) && !client.HasCapability( "STARTTLS" ) );

        const COM::CImapCommandResult* result = Wait( watch, client.Capability() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && result->capabilities.size() >= 3 );

        result = Wait( watch, client.List( "", "*" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->mailboxes.size() && result->mailboxes[ 1 ].name == "Sent" && result->mailboxes[ 0 ].hierarchyDelimiter == "/" );
        ASSERT_TRUE( result->mailboxes[ 0 ].attributes.size() == 1 && result->mailboxes[ 0 ].attributes[ 0 ] == "\\HasNoChildren" );

        result = Wait( watch, client.Select( "INBOX", false, 1000 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && result->code == "READ-WRITE" );
        ASSERT_TRUE( GUCEF_NULL != result && 3 == result->mailbox.exists && 1000 == result->mailbox.uidValidity && 4 == result->mailbox.uidNext && !result->mailbox.readOnly && !result->uidValidityChanged );
        ASSERT_TRUE( GUCEF_NULL != result && 5 == result->mailbox.flags.size() && result->mailbox.permanentFlags.size() >= 5 && result->mailbox.name == "INBOX" );
        ASSERT_TRUE( client.IsMailboxSelected() && 3 == client.GetSelectedMailbox().exists );

        result = Wait( watch, client.Select( "INBOX", true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && result->mailbox.readOnly && result->code == "READ-ONLY" );

        result = Wait( watch, client.Select( "Nowhere" ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->ok && result->status == "NO" && COM::MAILERR_PERMANENT == result->errorClass && result->code == "NONEXISTENT" && !client.IsMailboxSelected() );

        result = Wait( watch, client.Status( "INBOX" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->mailbox.exists && 4 == result->mailbox.uidNext && 1000 == result->mailbox.uidValidity );

        result = Wait( watch, client.Noop() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );

        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );
        ASSERT_TRUE( server.WaitForCommand( "LOGOUT", 5000 ) );                     // the server reads it on a thread of its own
        ASSERT_TRUE( HasCommand( server.GetCommands(), "LOGOUT" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: the LOGOUT of a client that closes at once reaches a server that is slow" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );

        // the NOOP is not answered yet when the client closes: the LOGOUT goes out right behind it, and the socket is closed with the answer still on its way
        server.replyDelayInMs = 300;
        ASSERT_TRUE( 0 != client.Noop() );
        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );

        // the server is still busy with the NOOP here, so a look right now sees neither: the test waits for the server
        ASSERT_TRUE( server.WaitForCommand( "NOOP", 5000 ) );
        ASSERT_TRUE( server.WaitForCommand( "LOGOUT", 5000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: authentication with PLAIN, SCRAM-SHA-256 and XOAUTH2 with and without the initial response, and a wrong password" )
    try
    {
        const char* mechanisms[] = { "PLAIN", "SCRAM-SHA-256", "XOAUTH2" };
        for ( int round=0; round<6; ++round )
        {
            const int m = round % 3;
            TESTFW::CTestImapServer server;
            server.allowLogin = false;
            server.authMechanisms = mechanisms[ m ];
            server.saslIr = round < 3;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.password = 2 == m ? server.token : server.password;
            settings.passwordIsToken = 2 == m;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, settings ) );
            const COM::CImapCommandResult* result = Wait( watch, client.Select( "INBOX" ) );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok );
            ASSERT_TRUE( HasCommand( server.GetCommands(), "AUTHENTICATE" ) && !HasCommand( server.GetCommands(), "LOGIN" ) );
            client.Disconnect();
        }
        {
            TESTFW::CTestImapServer server;
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.password = "wrong";
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) && PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_AUTH == watch.error.errorClass && !watch.ready );
        }
        {
            // not without TLS
            TESTFW::CTestImapServer server;
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.allowInsecureAuth = false;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) && PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
            ASSERT_FALSE( HasCommand( server.GetCommands(), "LOGIN" ) );
        }
        {
            // a login that is disabled before TLS, and no mechanism that is usable
            TESTFW::CTestImapServer server;
            server.loginDisabledBeforeTls = true;
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( LoginSettings( server ) ) && PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && !watch.ready );
            ASSERT_FALSE( HasCommand( server.GetCommands(), "LOGIN" ) );
        }
        {
            // PREAUTH needs no login
            TESTFW::CTestImapServer server;
            server.preauth = true;
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, PlainSettings( server ) ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: SEARCH and FETCH with the items that a client wants, and what a peek leaves alone" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

        COM::CImapSearchCriteria all;
        const COM::CImapCommandResult* result = Wait( watch, client.Search( all, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->numbers.size() && 3 == result->numbers[ 2 ] );

        COM::CImapSearchCriteria unseen;
        unseen.AddUnseen();
        result = Wait( watch, client.Search( unseen, false ) );
        ASSERT_TRUE( GUCEF_NULL != result && 2 == result->numbers.size() && 1 == result->numbers[ 0 ] && 2 == result->numbers[ 1 ] );

        COM::CImapSearchCriteria game;
        game.AddUidRange( 2, 0 );
        game.AddHeader( "X-Redux-Game", "g-9999" );
        result = Wait( watch, client.Search( game, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && 1 == result->numbers.size() && 2 == result->numbers[ 0 ] );

        COM::CImapSearchCriteria beyond;
        beyond.AddUidRange( 50, 0 );
        result = Wait( watch, client.Search( beyond, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && 1 == result->numbers.size() && 3 == result->numbers[ 0 ] );

        COM::CImapSearchCriteria header;
        header.AddHeader( "X-Redux-Game", "" );
        result = Wait( watch, client.Search( header, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && 2 == result->numbers.size() );

        COM::CImapFetchSpec spec;
        spec.flags = true;
        spec.size = true;
        spec.internalDate = true;
        spec.envelope = true;
        spec.bodyStructure = true;
        result = Wait( watch, client.Fetch( "1:3", spec, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->fetched.size() );
        if ( GUCEF_NULL != result && 3 == result->fetched.size() )
        {
            const COM::CImapFetchedItem& item = result->fetched[ 2 ];
            ASSERT_TRUE( 3 == item.uid && 3 == item.sequence && item.HasFlag( "\\Seen" ) && item.hasSize && item.size > 50 );
            ASSERT_TRUE( item.hasInternalDate && item.internalDate == "02-Nov-2020 10:11:12 +0000" && item.hasBodyStructure && item.bodyStructure.GetContentType() == "text/plain" );
            ASSERT_TRUE( !result->fetched[ 0 ].HasFlag( "\\Seen" ) );
        }

        // sections: header, text, the whole message, the fields of the header, and a part of it; a peek does not make the message seen
        COM::CImapFetchSpec sections;
        sections.flags = true;
        sections.sections.push_back( "HEADER" );
        sections.sections.push_back( "TEXT" );
        sections.sections.push_back( "" );
        sections.sections.push_back( "HEADER.FIELDS (SUBJECT X-REDUX-GAME)" );
        result = Wait( watch, client.Fetch( "1", sections, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 1 == result->fetched.size() );
        if ( GUCEF_NULL != result && 1 == result->fetched.size() )
        {
            const COM::CImapFetchedItem& item = result->fetched[ 0 ];
            ASSERT_TRUE( !item.HasFlag( "\\Seen" ) && 4 == item.sections.size() );
            ASSERT_TRUE( GUCEF_NULL != item.FindSection( "HEADER" ) && AsText( item.FindSection( "HEADER" )->data ).StartsWith( "From: Alice" ) && AsText( item.FindSection( "HEADER" )->data ).EndsWith( "\r\n\r\n" ) );
            ASSERT_TRUE( GUCEF_NULL != item.FindSection( "TEXT" ) && AsText( item.FindSection( "TEXT" )->data ) == "first body\r\nline two\r\n" );
            ASSERT_TRUE( GUCEF_NULL != item.FindSection( "" ) && AsText( item.FindSection( "" )->data ) == server.GetMessages()[ 0 ].data );
            const COM::CImapFetchSection* fields = item.FindSection( "HEADER.FIELDS (SUBJECT X-REDUX-GAME)" );
            ASSERT_TRUE( GUCEF_NULL != fields && AsText( fields->data ).HasSubstr( "Subject: one" ) >= 0 && AsText( fields->data ).HasSubstr( "X-Redux-Game: g-1234" ) >= 0 && AsText( fields->data ).HasSubstr( "From:" ) < 0 );
        }
        COM::CImapFetchSpec part;
        part.sections.push_back( "TEXT" );
        part.partialOffset = 6;
        part.partialLength = 4;
        result = Wait( watch, client.Fetch( "1", part, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && 1 == result->fetched.size() && result->fetched[ 0 ].sections.size() == 1 );
        ASSERT_TRUE( GUCEF_NULL != result && result->fetched[ 0 ].sections[ 0 ].hasOrigin && 6 == result->fetched[ 0 ].sections[ 0 ].origin && AsText( result->fetched[ 0 ].sections[ 0 ].data ) == "body" );

        // without the peek the message is seen
        COM::CImapFetchSpec seeing;
        seeing.markSeen = true;
        seeing.sections.push_back( "TEXT" );
        result = Wait( watch, client.Fetch( "2", seeing, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && 1 == result->fetched.size() && result->fetched[ 0 ].HasFlag( "\\Seen" ) );

        // a message that is not there, and a section that is not known to the server
        result = Wait( watch, client.Fetch( "77", spec, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 0 == result->fetched.size() );
        COM::CImapFetchSpec numbered;
        numbered.sections.push_back( "1.2" );
        result = Wait( watch, client.Fetch( "1", numbered, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && 1 == result->fetched.size() && result->fetched[ 0 ].sections[ 0 ].isNil );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: STORE, EXPUNGE, UID EXPUNGE, CLOSE, CREATE, DELETE and APPEND with a literal" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

        CORE::CString::StringVector flags;
        flags.push_back( "\\Deleted" );
        flags.push_back( "$Processed" );
        const COM::CImapCommandResult* result = Wait( watch, client.Store( "1,3", COM::IMAPSTORE_ADD, flags, true, false ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 2 == result->fetched.size() && result->fetched[ 0 ].HasFlag( "$Processed" ) && result->fetched[ 1 ].uid == 3 );
        result = Wait( watch, client.UidExpunge( "1" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 2 == server.GetMessages().size() && 2 == server.GetMessages()[ 0 ].uid );
        CORE::CString::StringVector only;
        only.push_back( "\\Flagged" );
        result = Wait( watch, client.Store( "2", COM::IMAPSTORE_REPLACE, only, true, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 0 == result->fetched.size() && 1 == server.GetMessages()[ 0 ].flags.size() );
        result = Wait( watch, client.Store( "2", COM::IMAPSTORE_REMOVE, only, true, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 0 == server.GetMessages()[ 0 ].flags.size() );
        result = Wait( watch, client.Expunge() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 1 == server.GetMessages().size() );

        // APPEND: a literal that has line breaks, the flags, UIDPLUS answers with the uid
        const CORE::CString data = RawMessage( "appended", "text\r\nwith lines\r\n.and a dot\r\n" );
        CORE::CString::StringVector seen;
        seen.push_back( "\\Seen" );
        result = Wait( watch, client.AppendRaw( "INBOX", data.C_String(), data.ByteSize() - 1, seen, "02-Nov-2020 10:11:12 +0000" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 4 == result->appendUid );
        ASSERT_TRUE( 2 == server.GetMessages().size() && server.GetMessages()[ 1 ].data == data && server.GetMessages()[ 1 ].flags.size() == 1 );
        COM::CMailMessage message = MakeMessage( "composed and appended", "body text" );
        result = Wait( watch, client.AppendMessage( "Sent", message ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 2 == server.GetAppended().size() );
        ASSERT_TRUE( server.GetAppended().back().data.HasSubstr( "Subject: composed and appended" ) >= 0 );

        // mailbox names go over the wire in the form of RFC 3501, and come back as text
        result = Wait( watch, client.CreateMailbox( "Entw\xC3\xBC" "rfe" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );
        ASSERT_TRUE( HasCommand( server.GetCommands(), "CREATE \"Entw&APw-rfe\"" ) );
        result = Wait( watch, client.List( "", "*" ) );
        ASSERT_TRUE( GUCEF_NULL != result && 4 == result->mailboxes.size() && result->mailboxes[ 3 ].name == "Entw\xC3\xBC" "rfe" );
        result = Wait( watch, client.DeleteMailbox( "Entw\xC3\xBC" "rfe" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok );

        result = Wait( watch, client.Close() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && !client.IsMailboxSelected() );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: COPY puts a copy of exactly the named messages in another folder, flags and COPYUID included, and leaves the originals" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

        const COM::CImapCommandResult* result = Wait( watch, client.Copy( "3", "Sent" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && COM::IMAPCMD_COPY == result->command );
        ASSERT_TRUE( 1 == result->copyUid );
        ASSERT_TRUE( 3 == server.GetMessages().size() );                        // the originals stay
        ASSERT_TRUE( 1 == server.GetFolder( "Sent" ).size() );
        ASSERT_TRUE( server.GetFolder( "Sent" )[ 0 ].data.HasSubstr( "Subject: three" ) >= 0 );
        ASSERT_TRUE( 1 == server.GetFolder( "Sent" )[ 0 ].flags.size() );        // the flags go with the copy
        ASSERT_TRUE( HasCommand( server.GetCommands(), "UID COPY 3 \"Sent\"" ) );

        // a set of messages
        result = Wait( watch, client.Copy( "1:2", "Drafts" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 2 == server.GetFolder( "Drafts" ).size() );

        // a folder that does not exist: the server says no, nothing is copied, the originals stay
        result = Wait( watch, client.Copy( "1", "Nowhere" ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->ok && result->code == "TRYCREATE" );
        ASSERT_TRUE( 3 == server.GetMessages().size() && 0 == server.GetFolder( "Nowhere" ).size() );

        // names and sets that could end a command are refused before they are sent
        const size_t commandsBefore = server.GetCommands().size();
        ASSERT_TRUE( 0 == client.Copy( "1\r\nA1 DELETE INBOX", "Sent" ) );
        ASSERT_TRUE( 0 == client.Copy( "1", "Sent\r\nA2 DELETE INBOX" ) );
        ASSERT_TRUE( 0 == client.Copy( "", "Sent" ) );
        ASSERT_TRUE( commandsBefore == server.GetCommands().size() );

        // without a selected mailbox the server refuses
        ASSERT_TRUE( Wait( watch, client.Close() )->ok );
        result = Wait( watch, client.Copy( "1", "Sent" ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->ok );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: MOVE (RFC 6851) is used when the server offers it, moves exactly the named messages and is refused without the capability" )
    try
    {
        {
            TESTFW::CTestImapServer server;
            server.allowMove = true;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
            ASSERT_TRUE( client.HasCapability( "MOVE" ) );
            ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

            const COM::CImapCommandResult* result = Wait( watch, client.Move( "2", "Sent" ) );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok && COM::IMAPCMD_MOVE == result->command );
            ASSERT_TRUE( 2 == server.GetMessages().size() && 1 == server.GetMessages()[ 0 ].uid && 3 == server.GetMessages()[ 1 ].uid );
            ASSERT_TRUE( 1 == server.GetFolder( "Sent" ).size() && server.GetFolder( "Sent" )[ 0 ].data.HasSubstr( "Subject: two" ) >= 0 );
            ASSERT_TRUE( HasCommand( server.GetCommands(), "UID MOVE 2 \"Sent\"" ) );
            ASSERT_TRUE( 2 == client.GetSelectedMailbox().exists );

            // a folder that does not exist: nothing moves
            result = Wait( watch, client.Move( "1", "Nowhere" ) );
            ASSERT_TRUE( GUCEF_NULL != result && !result->ok );
            ASSERT_TRUE( 2 == server.GetMessages().size() );
            client.Disconnect();
        }
        {
            TESTFW::CTestImapServer server;                 // no MOVE
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
            ASSERT_FALSE( client.HasCapability( "MOVE" ) );
            ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );
            const size_t commandsBefore = server.GetCommands().size();
            ASSERT_TRUE( 0 == client.Move( "1", "Sent" ) );
            ASSERT_TRUE( client.GetLastRefusal().HasSubstr( "MOVE" ) >= 0 );
            ASSERT_TRUE( commandsBefore == server.GetCommands().size() );
            ASSERT_TRUE( 3 == server.GetMessages().size() );
            client.Disconnect();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: COPY, STORE \\Deleted and UID EXPUNGE of one UID removes that message and no other message that carries the deleted flag" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( client.HasCapability( "UIDPLUS" ) );
        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

        // the player marked message 1 as deleted by hand; the game moves message 2
        CORE::CString::StringVector deleted;
        deleted.push_back( "\\Deleted" );
        ASSERT_TRUE( Wait( watch, client.Store( "1", COM::IMAPSTORE_ADD, deleted, true, true ) )->ok );
        ASSERT_TRUE( Wait( watch, client.Copy( "2", "Sent" ) )->ok );
        ASSERT_TRUE( Wait( watch, client.Store( "2", COM::IMAPSTORE_ADD, deleted, true, true ) )->ok );
        ASSERT_TRUE( Wait( watch, client.UidExpunge( "2" ) )->ok );
        ASSERT_TRUE( 2 == server.GetMessages().size() );
        ASSERT_TRUE( 1 == server.GetMessages()[ 0 ].uid && 3 == server.GetMessages()[ 1 ].uid );
        ASSERT_TRUE( 1 == server.GetMessages()[ 0 ].flags.size() );             // still marked, still there
        ASSERT_TRUE( HasCommand( server.GetCommands(), "UID EXPUNGE 2" ) );
        for ( size_t i=0; i<server.GetCommands().size(); ++i )                  // no plain EXPUNGE was ever sent
            ASSERT_FALSE( server.GetCommands()[ i ].HasSubstr( " EXPUNGE" ) >= 0 && server.GetCommands()[ i ].HasSubstr( "UID" ) < 0 );

        // a server without UIDPLUS: UID EXPUNGE is not sent at all
        server.allowUidPlus = false;
        COM::CImapClient other;
        CImapWatch otherWatch;
        otherWatch.Attach( other );
        ASSERT_TRUE( Connected( other, otherWatch, LoginSettings( server ) ) );
        ASSERT_FALSE( other.HasCapability( "UIDPLUS" ) );
        ASSERT_TRUE( Wait( otherWatch, other.Select( "INBOX" ) )->ok );
        const size_t commandsBefore = server.GetCommands().size();
        ASSERT_TRUE( 0 == other.UidExpunge( "3" ) );
        ASSERT_TRUE( commandsBefore == server.GetCommands().size() );
        other.Disconnect();
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: names, texts, flags and sets with line breaks or other tricks never become part of another command" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );
        const UInt32 before = (UInt32) server.GetCommands().size();

        ASSERT_TRUE( 0 == client.Select( "INBOX\r\nA9 DELETE INBOX" ) && !client.GetLastRefusal().IsNULLOrEmpty() );
        ASSERT_TRUE( 0 == client.CreateMailbox( "x\nA9 LOGOUT" ) );
        ASSERT_TRUE( 0 == client.List( "", "*\r\nA9 LOGOUT" ) );
        COM::CImapSearchCriteria evil;
        evil.AddSubject( "x\r\nA9 DELETE INBOX" );
        ASSERT_TRUE( 0 == client.Search( evil, true ) );
        COM::CImapFetchSpec spec;
        ASSERT_TRUE( 0 == client.Fetch( "1:*\r\nA9 DELETE INBOX", spec, true ) );
        ASSERT_TRUE( 0 == client.Fetch( "1:* BODY[]", spec, true ) );
        CORE::CString::StringVector flags;
        flags.push_back( "\\Seen)\r\nA9 DELETE INBOX" );
        ASSERT_TRUE( 0 == client.Store( "1", COM::IMAPSTORE_ADD, flags, true, true ) );
        flags.clear();
        flags.push_back( "\\Seen" );
        ASSERT_TRUE( 0 == client.Store( "1 2", COM::IMAPSTORE_ADD, flags, true, true ) );
        CORE::CString::StringVector badFlags;
        badFlags.push_back( "bad flag" );
        ASSERT_TRUE( 0 == client.AppendRaw( "INBOX", "x", 1, badFlags ) );
        ASSERT_TRUE( 0 == client.AppendRaw( "INBOX", "x", 1, CORE::CString::StringVector(), "date\"\r\nA9 DELETE INBOX" ) );
        PumpFor( 200 );
        ASSERT_TRUE( before == (UInt32) server.GetCommands().size() );

        // text that has to be a literal is one: the server sees the text, and the command is a single command
        COM::CImapSearchCriteria unicode;
        unicode.AddHeader( "Subject", "caf\xC3\xA9" );
        server.AddMessage( RawMessage( "caf\xC3\xA9 au lait", "text" ) );
        const COM::CImapCommandResult* result = Wait( watch, client.Noop() );
        ASSERT_TRUE( GUCEF_NULL != result );
        result = Wait( watch, client.Search( unicode, true ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && 1 == result->numbers.size() && 4 == result->numbers[ 0 ] );
        ASSERT_TRUE( 0 == CountCommands( server.GetCommands(), "DELETE" ) );

        // a quote and a backslash in a name are escaped
        result = Wait( watch, client.CreateMailbox( "My \"box\" \\ x" ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && HasCommand( server.GetCommands(), "CREATE \"My \\\"box\\\" \\\\ x\"" ) );

        // not connected
        client.Disconnect();
        ASSERT_TRUE( PumpUntilTrue( watch.disconnected, 3000 ) );
        ASSERT_TRUE( 0 == client.Noop() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: a changed UIDVALIDITY is reported, with the UIDs of before meaning nothing" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        const COM::CImapCommandResult* result = Wait( watch, client.Select( "INBOX", false, 1000 ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->uidValidityChanged && 1000 == result->mailbox.uidValidity );
        server.ResetMailbox( 2000 );
        server.AddMessage( RawMessage( "fresh", "text" ) );
        result = Wait( watch, client.Select( "INBOX", false, 1000 ) );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && result->uidValidityChanged && 2000 == result->mailbox.uidValidity && 1 == result->mailbox.exists );
        result = Wait( watch, client.Select( "INBOX", false, 2000 ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->uidValidityChanged );
        result = Wait( watch, client.Select( "INBOX" ) );
        ASSERT_TRUE( GUCEF_NULL != result && !result->uidValidityChanged );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: IDLE raises what happens in the mailbox, a command ends it, it is renewed, and without the capability it is refused" )
    try
    {
        {
            TESTFW::CTestImapServer server;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
            ASSERT_TRUE( 0 == client.Idle() );      // nothing selected
            ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

            const UInt32 idleId = client.Idle();
            ASSERT_TRUE( 0 != idleId && PumpUntilCondition( CounterAtLeast( watch.idleStartedCount, 1 ), 5000 ) && client.IsIdling() );

            server.AddMessage( RawMessage( "arrived", "text" ) );
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.updateCount, 1 ), 5000 ) );
            ASSERT_TRUE( COM::CImapIdleUpdate::KIND_EXISTS == watch.updates[ 0 ].kind && 4 == watch.updates[ 0 ].number );
            server.PushUntagged( "1 EXPUNGE" );
            server.PushUntagged( "2 FETCH (FLAGS (\\Seen \\Flagged) UID 2)" );
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.updateCount, 3 ), 5000 ) );
            ASSERT_TRUE( COM::CImapIdleUpdate::KIND_EXPUNGE == watch.updates[ 1 ].kind && 1 == watch.updates[ 1 ].number );
            ASSERT_TRUE( COM::CImapIdleUpdate::KIND_FLAGS == watch.updates[ 2 ].kind && 2 == watch.updates[ 2 ].number && watch.updates[ 2 ].hasUid && 2 == watch.updates[ 2 ].uid && 2 == watch.updates[ 2 ].flags.size() );

            // a command ends the IDLE first
            const UInt32 noopId = client.Noop();
            ASSERT_TRUE( 0 != noopId );
            const COM::CImapCommandResult* noop = Wait( watch, noopId );
            ASSERT_TRUE( GUCEF_NULL != noop && noop->ok );
            ASSERT_TRUE( GUCEF_NULL != watch.Find( idleId ) && watch.Find( idleId )->ok && !client.IsIdling() );
            ASSERT_TRUE( HasCommand( server.GetCommands(), "DONE" ) );

            // ended by the caller
            const UInt32 second = client.Idle();
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.idleStartedCount, 2 ), 5000 ) );
            client.IdleDone();
            ASSERT_TRUE( GUCEF_NULL != Wait( watch, second ) && watch.Find( second )->ok );
            client.Disconnect();
        }
        {
            // the IDLE is ended and started again before a server gives up on it
            TESTFW::CTestImapServer server;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.idleRenewInMs = 300;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, settings ) );
            ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );
            const UInt32 idleId = client.Idle();
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.idleStartedCount, 3 ), 8000 ) );
            ASSERT_TRUE( CountCommands( server.GetCommands(), " IDLE" ) >= 3 && CountCommands( server.GetCommands(), "DONE" ) >= 2 );
            // renewals are not results: the command has not completed
            ASSERT_TRUE( GUCEF_NULL == watch.Find( idleId ) && client.IsIdling() );
            server.AddMessage( RawMessage( "late", "text" ) );
            ASSERT_TRUE( PumpUntilCondition( CounterAtLeast( watch.updateCount, 1 ), 5000 ) );
            client.Disconnect();
        }
        {
            TESTFW::CTestImapServer server;
            server.allowIdle = false;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
            ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );
            ASSERT_TRUE( 0 == client.Idle() && !client.GetLastRefusal().IsNULLOrEmpty() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: an IDLE that is asked to end before the server has confirmed it ends as soon as the server has, and is not left idling for ever" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );

        // a slow server: the IDLE is on its way and not yet confirmed when the caller wants it to end
        server.replyDelayInMs = 300;
        const UInt32 idleId = client.Idle();
        ASSERT_TRUE( 0 != idleId && !client.IsIdling() );
        client.IdleDone();
        const COM::CImapCommandResult* idle = Wait( watch, idleId, 5000 );
        ASSERT_TRUE( GUCEF_NULL != idle && idle->ok );
        ASSERT_TRUE( !client.IsIdling() && HasCommand( server.GetCommands(), "DONE" ) );

        // the client is usable afterwards
        server.replyDelayInMs = 0;
        ASSERT_TRUE( Wait( watch, client.Noop() )->ok );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: STARTTLS, TLS from the start, LOGIN that is allowed after TLS only, STARTTLS that is required and not offered" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !tlsReady, "TLS cannot be tested here" );
        {
            TESTFW::CTestImapServer server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.offerStartTls = true;
            server.loginDisabledBeforeTls = true;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, settings ) );
            GUCEF::vector< CORE::CString > commands = server.GetCommands();
            ASSERT_TRUE( commands.size() >= 4 && commands[ 0 ].HasSubstr( "CAPABILITY" ) >= 0 && commands[ 1 ].HasSubstr( "STARTTLS" ) >= 0 && commands[ 2 ].HasSubstr( "CAPABILITY" ) >= 0 );
            const COM::CImapCommandResult* result = Wait( watch, client.Select( "INBOX" ), 15000 );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok && 3 == result->mailbox.exists );
            client.Disconnect();
        }
        {
            TESTFW::CTestImapServer server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.SetImplicitTls( true );
            server.allowLogin = false;
            server.authMechanisms = "SCRAM-SHA-256 PLAIN";
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_IMPLICIT );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, settings ) );
            client.Disconnect();
        }
        {
            TESTFW::CTestImapServer server;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) && PumpUntilTrue( watch.settled, 8000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
        }
        {
            TESTFW::CTestImapServer server;
            server.SetTlsFiles( certs->GetPath( "good.crt" ), certs->GetPath( "good.key" ) );
            server.offerStartTls = true;
            server.hostile = TESTFW::CTestImapServer::HOSTILE_INJECT_AFTER_STARTTLS;
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_STARTTLS );
            settings.userName = "alice";
            settings.password = server.password;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) && PumpUntilTrue( watch.settled, 15000 ) );
            ASSERT_TRUE( watch.failed && !watch.ready );
            ASSERT_FALSE( HasCommand( server.GetCommands(), "LOGIN" ) );
        }
        {
            // a certificate for another authority
            TESTFW::CTestImapServer server;
            server.SetTlsFiles( certs->GetPath( "otherca_leaf.crt" ), certs->GetPath( "otherca_leaf.key" ) );
            server.SetImplicitTls( true );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings;
            MakeSecure( settings, server.GetPort(), *certs, COM::MAILTLS_IMPLICIT );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) && PumpUntilTrue( watch.settled, 15000 ) );
            ASSERT_TRUE( watch.failed && COM::MAILERR_TLS == watch.error.errorClass );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: what is new is fetched and parsed from the UID that was last seen, and a new UIDVALIDITY starts over" )
    try
    {
        TESTFW::CTestImapServer server;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClientSettings settings = LoginSettings( server );

        COM::CImapFetchOptions options;
        COM::CImapFetchResult result;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( 3 == result.messages.size() && 1000 == result.uidValidity && 3 == result.highestUid && 4 == result.uidNext && !result.uidValidityChanged );
        ASSERT_TRUE( result.messages[ 0 ].parsed && result.messages[ 0 ].message.subject == "one" && result.messages[ 0 ].message.textBody == "first body\nline two" );
        ASSERT_TRUE( result.messages[ 2 ].uid == 3 && result.messages[ 0 ].size > 50 );
        // nothing was marked: the fetch only looked
        ASSERT_TRUE( 0 == server.GetMessages()[ 0 ].flags.size() );

        options.lastUidValidity = result.uidValidity;
        options.lastUid = result.highestUid;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( 0 == result.messages.size() && !result.uidValidityChanged );

        server.AddMessage( RawMessage( "four", "fourth\r\n", "X-Redux-Game: g-1234\r\n" ) );
        server.AddMessage( RawMessage( "five", "fifth\r\n", "X-Redux-Game: g-9999\r\n" ) );
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( 2 == result.messages.size() && 4 == result.messages[ 0 ].uid && 5 == result.highestUid );

        // only what the server finds for the header, at most one
        options.headerNames.push_back( "X-Redux-Game" );
        options.headerTexts.push_back( "g-9999" );
        options.lastUid = 0;
        options.lastUidValidity = 0;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( 2 == result.messages.size() && 2 == result.messages[ 0 ].uid && 5 == result.messages[ 1 ].uid );
        options.maxMessages = 1;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) && 1 == result.messages.size() && 2 == result.highestUid );
        options.headerNames.clear();
        options.headerTexts.clear();
        options.maxMessages = 0;

        // marked as seen, then deleted
        options.markSeen = true;
        options.lastUid = 3;
        options.lastUidValidity = 1000;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) && 2 == result.messages.size() );
        GUCEF::vector< TESTFW::CTestImapServer::TMessage > stored = server.GetMessages();
        ASSERT_TRUE( 5 == stored.size() && 1 == stored[ 3 ].flags.size() && stored[ 3 ].flags[ 0 ] == "\\Seen" && 1 == stored[ 4 ].flags.size() );
        options.markSeen = false;
        options.deleteAfterFetch = true;
        options.lastUid = 0;
        options.lastUidValidity = 0;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) && 5 == result.messages.size() );
        ASSERT_TRUE( 0 == server.GetMessages().size() );
        options.deleteAfterFetch = false;

        // the mailbox was made again: the cursor of before is no use, everything is looked at
        server.ResetMailbox( 5000 );
        server.AddMessage( RawMessage( "new one", "text\r\n" ) );
        options.lastUidValidity = 1000;
        options.lastUid = 5;
        ASSERT_TRUE( COM::CImapClient::FetchNewSync( settings, options, result ) );
        ASSERT_TRUE( result.uidValidityChanged && 5000 == result.uidValidity && 1 == result.messages.size() && 1 == result.messages[ 0 ].uid );

        // the pool thread
        CORE::CFutureResult future = COM::CMailOperations::FetchNewImapAsync( settings, COM::CImapFetchOptions() );
        ASSERT_TRUE( future.HasAFuture() );
        future.Await( 30000 );
        COM::CImapFetchResult asyncResult;
        ASSERT_TRUE( COM::CMailOperations::TryGetImapFetchResult( future, asyncResult ) && asyncResult.success && 1 == asyncResult.messages.size() );

        // not there at all, and a wrong password
        COM::CImapClientSettings refused;
        MakePlain( refused, TESTFW::CTlsTestServer::FindFreePort() );
        ASSERT_FALSE( COM::CImapClient::FetchNewSync( refused, COM::CImapFetchOptions(), result ) );
        ASSERT_TRUE( COM::MAILERR_NONE != result.errorClass );
        COM::CImapClientSettings wrong = settings;
        wrong.password = "wrong";
        ASSERT_FALSE( COM::CImapClient::FetchNewSync( wrong, COM::CImapFetchOptions(), result ) );
        ASSERT_TRUE( COM::MAILERR_AUTH == result.errorClass );
        COM::CImapFetchOptions noBox;
        noBox.mailbox = "Nowhere";
        ASSERT_FALSE( COM::CImapClient::FetchNewSync( settings, noBox, result ) );
        ASSERT_TRUE( COM::MAILERR_PERMANENT == result.errorClass );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: a server that drops the connection or is silent ends the commands that wait with an error" )
    try
    {
        {
            TESTFW::CTestImapServer server;
            FillMailbox( server );
            server.dropAfterCommands = 3;
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
            const UInt32 first = client.Select( "INBOX" );
            const UInt32 second = client.Noop();
            const UInt32 third = client.Noop();
            ASSERT_TRUE( PumpUntilTrue( watch.failed, 8000 ) );
            ASSERT_TRUE( COM::MAILERR_CONNECTION == watch.error.errorClass );
            ASSERT_TRUE( GUCEF_NULL != Wait( watch, first, 1000 ) && GUCEF_NULL != Wait( watch, third, 1000 ) );
            ASSERT_TRUE( !watch.Find( third )->ok && COM::MAILERR_CONNECTION == watch.Find( third )->errorClass );
            (void) second;
        }
        {
            TESTFW::CTestImapServer server;
            FillMailbox( server );
            server.replyDelayInMs = 1500;
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.commandTimeoutInMs = 400;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( client.Connect( settings ) && PumpUntilTrue( watch.failed, 8000 ) );
            ASSERT_TRUE( COM::MAILERR_TIMEOUT == watch.error.errorClass );
        }
        {
            TESTFW::CTestImapServer server;
            server.hostile = TESTFW::CTestImapServer::HOSTILE_BYE_ON_SELECT;
            FillMailbox( server );
            ASSERT_TRUE( server.Start() );
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
            const UInt32 id = client.Select( "INBOX" );
            ASSERT_TRUE( PumpUntilTrue( watch.failed, 8000 ) && COM::MAILERR_CONNECTION == watch.error.errorClass );
            ASSERT_TRUE( GUCEF_NULL != Wait( watch, id, 1000 ) && !watch.Find( id )->ok );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    {
        struct SHostileCase
        {
            TESTFW::CTestImapServer::EHostile mode;
            const char* name;
            bool atGreeting;
            bool fetchAfterSelect;
            COM::EMailErrorClass expected;
        };
        const SHostileCase cases[] = {
            { TESTFW::CTestImapServer::HOSTILE_GARBAGE, "a greeting that is not IMAP", true, false, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_GIANT_LINE, "a line that does not end", true, false, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_HUGE_LITERAL, "a literal of 4 GiB", false, true, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_LARGE_LITERAL, "a literal of 200 MiB", false, true, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_DEEP_NESTING, "lists nested a thousand deep", false, true, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_LITERAL_STALL, "a literal that does not complete", false, true, COM::MAILERR_TIMEOUT },
            { TESTFW::CTestImapServer::HOSTILE_ENDLESS_UNTAGGED, "responses that do not stop", false, false, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_NUMBER_OVERFLOW, "a number that does not fit", false, false, COM::MAILERR_PROTOCOL },
            { TESTFW::CTestImapServer::HOSTILE_WRONG_TAG, "a completion of a command that was not given", true, false, COM::MAILERR_PROTOCOL }
        };
        for ( size_t i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
        {
            GUCEF_TESTFW_TESTCASE( ( CORE::CString( "ImapClient: a hostile server cannot make the client read more than the limits: " ) + cases[ i ].name ).C_String() )
            try
            {
                TESTFW::CTestImapServer server;
                server.hostile = cases[ i ].mode;
                FillMailbox( server );
                ASSERT_TRUE( server.Start() );
                COM::CImapClientSettings settings = LoginSettings( server );
                settings.maxLineLength = 4096;
                settings.maxResponseCount = 500;
                settings.commandTimeoutInMs = 800;
                COM::CImapClient client;
                CImapWatch watch;
                watch.Attach( client );
                ASSERT_TRUE( client.Connect( settings ) );
                if ( !cases[ i ].atGreeting )
                {
                    ASSERT_TRUE( PumpUntilTrue( watch.ready, 8000 ) );
                    if ( cases[ i ].fetchAfterSelect )
                    {
                        // the hostile answer is to the fetch
                        server.hostile = TESTFW::CTestImapServer::HOSTILE_NONE;
                        ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );
                        server.hostile = cases[ i ].mode;
                        COM::CImapFetchSpec spec;
                        spec.sections.push_back( "" );
                        client.Fetch( "1", spec, true );
                    }
                    else
                        client.Select( "INBOX" );
                }
                ASSERT_TRUE( PumpUntilTrue( watch.failed, 15000 ) );
                ASSERT_TRUE( cases[ i ].expected == watch.error.errorClass );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
            GUCEF_TESTFW_TESTCASE_END
        }
    }

    GUCEF_TESTFW_TESTCASE( "ImapClient: the text of a status response that ends in {5} is text, and not the start of a literal" )
    try
    {
        TESTFW::CTestImapServer server;
        server.hostile = TESTFW::CTestImapServer::HOSTILE_STATUS_TEXT_WITH_BRACES;
        FillMailbox( server );
        ASSERT_TRUE( server.Start() );
        COM::CImapClient client;
        CImapWatch watch;
        watch.Attach( client );
        ASSERT_TRUE( Connected( client, watch, LoginSettings( server ) ) );
        const COM::CImapCommandResult* result = Wait( watch, client.Noop() );
        ASSERT_TRUE( GUCEF_NULL != result && result->ok && result->text.HasSubstr( "{5}" ) >= 0 );
        client.Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImapClient: the password, the user, the token and the content of messages are not in the log" )
    try
    {
        CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
        const Int32 previousLevel = logManager.GetMinLogLevel();
        logManager.SetMinLogLevel( CORE::LOGLEVEL_EVERYTHING );
        TESTFW::CLogTextCapture capture;
        {
            TESTFW::CTestImapServer server;
            server.password = "UNIQUE-IMAP-PASSWORD-7741";
            server.user = "alice-unique-imap-user";
            server.AddMessage( RawMessage( "UNIQUE-IMAP-SUBJECT-1188", "UNIQUE-IMAP-BODY-2299\r\n" ) );
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.userName = server.user;
            settings.password = server.password;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, settings ) );
            ASSERT_TRUE( Wait( watch, client.Select( "INBOX" ) )->ok );
            COM::CImapFetchSpec spec;
            spec.envelope = true;
            spec.sections.push_back( "" );
            const COM::CImapCommandResult* result = Wait( watch, client.Fetch( "1", spec, true ) );
            ASSERT_TRUE( GUCEF_NULL != result && result->ok && 1 == result->fetched.size() );
            client.Disconnect();
        }
        {
            TESTFW::CTestImapServer server;
            server.allowLogin = false;
            server.authMechanisms = "XOAUTH2";
            server.token = "ya29.UNIQUE-IMAP-TOKEN-5530";
            ASSERT_TRUE( server.Start() );
            COM::CImapClientSettings settings = LoginSettings( server );
            settings.password = server.token;
            settings.passwordIsToken = true;
            COM::CImapClient client;
            CImapWatch watch;
            watch.Attach( client );
            ASSERT_TRUE( Connected( client, watch, settings ) );
            client.Disconnect();
        }
        const CORE::CString text = capture.GetText();
        logManager.SetMinLogLevel( previousLevel );
        ASSERT_TRUE( text.HasSubstr( "IMAP 127.0.0.1 C: A0001 CAPABILITY" ) >= 0 && text.HasSubstr( "LOGIN ***" ) >= 0 && text.HasSubstr( "AUTHENTICATE XOAUTH2 ***" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "UNIQUE-IMAP-PASSWORD-7741" ) < 0 && text.HasSubstr( "alice-unique-imap-user" ) < 0 && text.HasSubstr( "UNIQUE-IMAP-TOKEN-5530" ) < 0 );
        ASSERT_TRUE( text.HasSubstr( "UNIQUE-IMAP-SUBJECT-1188" ) < 0 && text.HasSubstr( "UNIQUE-IMAP-BODY-2299" ) < 0 && text.HasSubstr( "alice@example.org" ) < 0 );
        const CORE::CString oauthMessage = "user=alice\x01" "auth=Bearer ya29.UNIQUE-IMAP-TOKEN-5530\x01\x01";
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
