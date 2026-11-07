/*
 *  gucefMULTIPLAY_MailProvider_TestApp: Application for testing the mail match provider of gucefMULTIPLAY
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

#include "gucefCOM_CMimeComposer.h"
#include "TestMailFixture.h"
#include "TestMailTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MULTIPLAYPLUGIN;
using namespace GUCEF::MAILTEST;

#define ERRORHERE       MAIL_ERRORHERE
#define ASSERT_TRUE(t)  MAIL_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) MAIL_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUCEF::vector< CORE::CString >
OneAddress( const char* a )
{GUCEF_TRACE;

    GUCEF::vector< CORE::CString > list;
    list.push_back( a );
    return list;
}

/*-------------------------------------------------------------------------*/

CORE::CString
PlainMail( const char* from, const char* to, const char* subject, const char* id )
{GUCEF_TRACE;

    return CORE::CString( "From: <" ) + from + ">\r\nTo: <" + to + ">\r\nSubject: " + subject + "\r\nMessage-ID: <" + id + "@plain.test>\r\n\r\nNothing about a game.\r\n";
}

/*-------------------------------------------------------------------------*/

/** A mail with the headers of a match that carries no data, of the kind: the cheapest mail the provider reads and remembers */
CORE::CString
AckMail( const CORE::CString& matchId, UInt32 revision, const char* id )
{GUCEF_TRACE;

    MAIL::CMailOutgoing outgoing;
    outgoing.kind = MAIL::MAILKIND_ACK;
    outgoing.matchId = matchId;
    outgoing.revision = revision;
    outgoing.title = "x";
    outgoing.from = COM::CMailAddress( "bob@mail.test" );
    outgoing.to.push_back( COM::CMailAddress( "alice@mail.test" ) );
    COM::CMailMessage message;
    CORE::CString reason;
    if ( !MAIL::CMailMatchCodec::Compose( outgoing, message, reason ) )
        return CORE::CString::Empty;
    message.messageId = CORE::CString( "<" ) + id + "@acks.test>";
    COM::CMimeComposer composer;
    CORE::CDynamicBuffer wire;
    if ( !composer.Compose( message, wire, reason ) )
        return CORE::CString::Empty;
    return CORE::CString( static_cast< const char* >( wire.GetConstBufferPtr() ), wire.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

UInt32
CountCommands( const GUCEF::vector< CORE::CString >& commands, const char* text )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( commands[ i ].HasSubstr( text ) >= 0 )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
StringContains( const CORE::CString& text, const char* part )
{GUCEF_TRACE;

    return text.HasSubstr( part ) >= 0;
}


/** What a run of a match through a mailbox with an action for processed mail left behind in the mailbox of the second player */
struct SActionRun
{
    bool ran;
    GUCEF::vector< TESTFW::CTestImapServer::TMessage > inbox;
    GUCEF::vector< TESTFW::CTestImapServer::TMessage > folder;
    GUCEF::vector< TESTFW::CTestImapServer::TMessage > appended;
    GUCEF::vector< CORE::CString > commands;
    GUCEF::vector< MAIL::CMailMatchNotice > problems;
    UInt32 matchMailsSent;
    SActionRun( void ) : ran( false ), inbox(), folder(), appended(), commands(), problems(), matchMailsSent( 0 ) {}
};

/*-------------------------------------------------------------------------*/

bool
IsMatchMail( const TESTFW::CTestImapServer::TMessage& message )
{GUCEF_TRACE;

    return message.data.HasSubstr( "X-Redux-Match:" ) >= 0;
}

/*-------------------------------------------------------------------------*/

bool
HasFlagNamed( const TESTFW::CTestImapServer::TMessage& message, const char* flag )
{GUCEF_TRACE;

    for ( size_t f=0; f<message.flags.size(); ++f )
        if ( message.flags[ f ].Equals( flag, false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  Three letters that are not about a game, one of them flagged \Deleted by the player, then a match of two players in which the
 *  second one receives an invitation, a turn and acknowledgements; the second player has 'action' for processed mail.
 */
void
RunActionScenario( SActionRun& run, const char* name, MAIL::TMailProcessedAction action, bool allowMove, bool allowUidPlus, bool folderExists, const char* folder = "GameMail" )
{GUCEF_TRACE;

    CMailTable table;
    if ( !table.Setup( 2, name ) )
        return;
    TESTFW::CTestImapServer& imap = table.P( 1 ).box->imap;
    imap.allowMove = allowMove;
    imap.allowUidPlus = allowUidPlus;
    if ( folderExists )
        imap.mailboxes.push_back( folder );
    table.P( 1 ).settings.processedAction = action;
    table.P( 1 ).settings.processedFolder = folder;
    if ( !table.P( 1 ).provider->SetSettings( table.P( 1 ).settings ) )
        return;
    GUCEF::vector< CORE::CString > deleted;
    deleted.push_back( "\\Deleted" );
    imap.AddMessage( PlainMail( "friend@mail.test", "bob@mail.test", "Old letter the player marked", "old1" ), deleted );
    imap.AddMessage( PlainMail( "friend@mail.test", "bob@mail.test", "Lunch?", "keep1" ) );
    imap.AddMessage( PlainMail( "list@news.test", "bob@mail.test", "[Redux 12345678 r1 t1] looks like a game but is not", "keep2" ) );
    if ( !table.StartAll() )
        return;
    MULTIPLAY::CMatchRecord record = table.MakeRecord( "Action game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
    MULTIPLAY::CMatchOperationResult result;
    if ( !table.CreateMatch( 0, record, 1, result ) || !table.WaitKnown( 1, record.info.matchId ) || !table.Respond( 1, record.info.matchId, true, result ) )
        return;
    if ( !table.PlayTurn( 1, record.info.matchId, 0, "b1", result ) || !table.WaitTrusted( 0, record.info.matchId, 2 ) )
        return;
    if ( !table.PlayTurn( 0, record.info.matchId, 1, "a1", result ) || !table.WaitTrusted( 1, record.info.matchId, 3 ) )
        return;
    table.Settle( 800 );
    run.inbox = imap.GetMessages();
    run.folder = imap.GetFolder( folder );
    run.appended = imap.GetAppended();
    run.commands = imap.GetCommands();
    run.problems = table.P( 1 ).recorder.problems;
    run.ran = true;
}

/*-------------------------------------------------------------------------*/

UInt32
CountMatchMailsIn( const GUCEF::vector< TESTFW::CTestImapServer::TMessage >& messages )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<messages.size(); ++i )
        if ( IsMatchMail( messages[ i ] ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

/** The three letters of the player are still there, with their flags as they were: only the old one is marked deleted, none is read */
bool
UnrelatedMailUntouched( const GUCEF::vector< TESTFW::CTestImapServer::TMessage >& inbox )
{GUCEF_TRACE;

    UInt32 unrelated = 0;
    for ( size_t i=0; i<inbox.size(); ++i )
    {
        if ( IsMatchMail( inbox[ i ] ) )
            continue;
        ++unrelated;
        const bool old = inbox[ i ].data.HasSubstr( "Subject: Old letter" ) >= 0;
        if ( HasFlagNamed( inbox[ i ], "\\Deleted" ) != old || HasFlagNamed( inbox[ i ], "\\Seen" ) )
            return false;
    }
    return 3 == unrelated;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailTransportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: the mailbox, IMAP and POP3" );

    GUCEF_TESTFW_TESTCASE( "IMAP with IDLE: the mailbox is watched by a persistent connection, a mail is seen as it arrives, and only mail of matches is asked for" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "IMAP with IDLE: the mailbox is watched by a persistent connection, a mail is seen as it arrives, and only mail of matches is asked for" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-idle" ) && table.StartAll() );
        bool idling = false;
        PUMP_WAIT( idling, table.P( 1 ).provider->GetInbox().IsIdling() && table.P( 0 ).provider->GetInbox().IsIdling(), 8000 );
        ASSERT_TRUE( idling && table.P( 1 ).provider->GetInbox().IsOnline() );
        ASSERT_TRUE( !table.P( 1 ).recorder.states.empty() && table.P( 1 ).recorder.states[ 0 ] );

        // no clock moves: the mail is seen because the server says it is there
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Idle game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, record.info.matchId, 3000 ) );
        GUCEF::vector< CORE::CString > commands = table.P( 1 ).box->imap.GetCommands();
        ASSERT_TRUE( CountCommands( commands, "IDLE" ) >= 1 && CountCommands( commands, "DONE" ) >= 1 );
        ASSERT_TRUE( CountCommands( commands, "SEARCH" ) >= 1 && CountCommands( commands, "X-Redux-Match" ) >= 1 );
        ASSERT_TRUE( 0 == CountCommands( commands, "EXPUNGE" ) && 0 == CountCommands( commands, "\\Deleted" ) );
        ASSERT_TRUE( table.P( 1 ).provider->GetInbox().GetHandledCount() >= 1 && table.P( 1 ).provider->GetInbox().GetLastUid() >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IMAP with IDLE: a request to look at the mailbox that comes while the server has not yet confirmed the IDLE does not end the watching, mail that comes later is still seen" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "IMAP with IDLE: a request to look at the mailbox that comes while the server has not yet confirmed the IDLE does not end the watching, mail that comes later is still seen" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-idle-late" ) && table.StartAll() );
        bool idling = false;
        PUMP_WAIT( idling, table.P( 1 ).provider->GetInbox().IsIdling() && table.P( 0 ).provider->GetInbox().IsIdling(), 8000 );
        ASSERT_TRUE( idling );

        // a slow server: the watching starts again after a request, and while the IDLE of the new round is on its way, a second request comes
        table.P( 1 ).box->imap.replyDelayInMs = 300;
        const UInt32 idlesBefore = CountCommands( table.P( 1 ).box->imap.GetCommands(), " IDLE" );
        ASSERT_TRUE( table.P( 1 ).provider->RefreshAll() );
        bool onItsWay = false;
        PUMP_WAIT( onItsWay, CountCommands( table.P( 1 ).box->imap.GetCommands(), " IDLE" ) > idlesBefore, 8000 );
        ASSERT_TRUE( onItsWay && !table.P( 1 ).provider->GetInbox().IsIdling() );
        ASSERT_TRUE( table.P( 1 ).provider->RefreshAll() );
        table.Settle( 100 );
        table.P( 1 ).box->imap.replyDelayInMs = 0;

        // the clock does not move: the mail is seen because the server says it is there, so the watching has to be going on
        bool watching = false;
        PUMP_WAIT( watching, table.P( 1 ).provider->GetInbox().IsIdling(), 8000 );
        ASSERT_TRUE( watching );
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Late idle game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, record.info.matchId, 5000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IMAP without IDLE: the mailbox is polled at the interval with a NOOP and a search" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "IMAP without IDLE: the mailbox is polled at the interval with a NOOP and a search" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-poll" ) );
        table.P( 0 ).box->imap.allowIdle = false;
        table.P( 1 ).box->imap.allowIdle = false;
        ASSERT_TRUE( table.StartAll() );
        table.Settle( 600 );
        ASSERT_FALSE( table.P( 1 ).provider->GetInbox().IsIdling() );
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Polled game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) );
        // the mail is there and is not seen before the interval has passed
        bool delivered = false;
        PUMP_WAIT( delivered, table.CountMatchMails( 1, "INVITE" ) >= 1, 8000 );
        ASSERT_TRUE( delivered );
        table.Settle( 400 );
        MULTIPLAY::CMatchInfo info;
        ASSERT_FALSE( table.P( 1 ).provider->GetMatchInfo( record.info.matchId, info ) );
        bool known = false;
        for ( int i=0; i<30 && !known; ++i )
        {
            table.Advance( 600, 150 );
            known = table.P( 1 ).provider->GetMatchInfo( record.info.matchId, info );
        }
        ASSERT_TRUE( known );
        GUCEF::vector< CORE::CString > commands = table.P( 1 ).box->imap.GetCommands();
        ASSERT_TRUE( CountCommands( commands, "NOOP" ) >= 1 && CountCommands( commands, "SEARCH" ) >= 2 && 0 == CountCommands( commands, " IDLE" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "POP3: a poll lists the unique ids, reads the headers of the new mail and the whole of mail of matches once, and deletes nothing" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "POP3: a poll lists the unique ids, reads the headers of the new mail and the whole of mail of matches once, and deletes nothing" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions options;
        options.useImap = false;
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-pop3", options ) && table.StartAll() );
        // three letters that are not about a game
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "friend@mail.test", "bob@mail.test", "Lunch?", "l1" ) ) );
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "friend@mail.test", "bob@mail.test", "[Redux 12345678 r1 t1] looks like it but is not", "l2" ) ) );
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "friend@mail.test", "bob@mail.test", "Photos", "l3" ) ) );
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Pop game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) );
        bool delivered = false;
        PUMP_WAIT( delivered, table.CountMatchMails( 1, "INVITE" ) >= 1, 8000 );
        ASSERT_TRUE( delivered );
        bool known = false;
        for ( int i=0; i<40 && !known; ++i )
        {
            table.Advance( 600, 150 );
            MULTIPLAY::CMatchInfo info;
            known = table.P( 1 ).provider->GetMatchInfo( record.info.matchId, info );
        }
        ASSERT_TRUE( known );
        // more polls, nothing is read twice
        for ( int i=0; i<6; ++i )
            table.Advance( 600, 150 );
        GUCEF::vector< CORE::CString > commands = table.P( 1 ).box->pop3.GetCommands();
        ASSERT_TRUE( CountCommands( commands, "UIDL" ) >= 2 );
        ASSERT_TRUE( CountCommands( commands, "TOP 1" ) <= 1 && CountCommands( commands, "TOP 2" ) <= 1 && CountCommands( commands, "TOP 3" ) <= 1 );
        ASSERT_TRUE( 0 == CountCommands( commands, "RETR 1" ) && 0 == CountCommands( commands, "RETR 2" ) && 0 == CountCommands( commands, "RETR 3" ) );
        ASSERT_TRUE( 1 == CountCommands( commands, "RETR 4" ) );
        ASSERT_TRUE( 0 == CountCommands( commands, "DELE" ) && 0 == table.P( 1 ).box->pop3.GetCommittedDeletes() );
        // a turn goes the same way
        ASSERT_TRUE( table.Respond( 1, record.info.matchId, true, result ) );
        bool active = false;
        for ( int i=0; i<40 && !active; ++i )
        {
            table.Advance( 600, 150 );
            active = MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( 0, record.info.matchId );
        }
        ASSERT_TRUE( active );
        ASSERT_TRUE( table.PlayTurn( 1, record.info.matchId, 0, "p1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        bool second = false;
        for ( int i=0; i<40 && !second; ++i )
        {
            table.Advance( 600, 150 );
            second = table.P( 0 ).provider->GetTrustedRevision( record.info.matchId ) >= 2;
        }
        ASSERT_TRUE( second );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UIDVALIDITY: when the mailbox is a new one the position in the UIDs starts again, and the mails that are known are not read again" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "UIDVALIDITY: when the mailbox is a new one the position in the UIDs starts again, and the mails that are known are not read again" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions options;
        options.autoClockMs = 25;
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-uidvalidity", options ) );
        table.P( 0 ).box->imap.allowIdle = false;
        table.P( 1 ).box->imap.allowIdle = false;
        ASSERT_TRUE( table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 0, "u1", result ) && table.WaitTrusted( 0, id, 2 ) );
        table.Settle( 300 );
        MAIL::CMailInbox const& inbox = table.P( 0 ).provider->GetInbox();
        ASSERT_TRUE( 1000 == inbox.GetUidValidity() && inbox.GetLastUid() >= 2 );
        const UInt32 updatesBefore = (UInt32) table.P( 0 ).recorder.updated.size();

        // the server loses the mailbox and makes it again with the same mails in it, as backups do
        GUCEF::vector< TESTFW::CTestImapServer::TMessage > old = table.P( 0 ).box->imap.GetMessages();
        ASSERT_TRUE( old.size() >= 2 );
        table.P( 0 ).box->imap.ResetMailbox( 7777 );
        for ( size_t i=0; i<old.size(); ++i )
            table.P( 0 ).box->imap.AddMessage( old[ i ].data );
        // and a new turn comes: it gets a UID that is lower than the position that was kept
        ASSERT_TRUE( table.PlayTurn( 0, id, 1, "u2", result ) && table.WaitTrusted( 1, id, 3 ) );
        ASSERT_TRUE( table.PlayTurn( 1, id, 0, "u3", result ) );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 4 ) );
        ASSERT_TRUE( 7777 == inbox.GetUidValidity() );
        // the old mails were seen as old: nothing was applied twice, nothing was refused
        ASSERT_TRUE( 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) && 0 == table.P( 0 ).recorder.refused.size() );
        ASSERT_TRUE( table.P( 0 ).recorder.updated.size() <= updatesBefore + 2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unrelated mail: a mailbox full of other mail is never fetched, flagged or touched" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Unrelated mail: a mailbox full of other mail is never fetched, flagged or touched" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-unrelated" ) );
        for ( int i=0; i<120; ++i )
            ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "list@news.test", "bob@mail.test", i % 7 == 0 ? "[Redux 12345678 r1 t1] a subject that only looks like a game" : "Newsletter", ( "n" + CORE::UInt32ToString( i ) ).C_String() ) ) );
        ASSERT_TRUE( table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 0, "n1", result ) && table.WaitTrusted( 0, id, 2 ) );

        GUCEF::vector< TESTFW::CTestImapServer::TMessage > mails = table.P( 1 ).box->imap.GetMessages();
        GUCEF::vector< CORE::CString > commands = table.P( 1 ).box->imap.GetCommands();
        UInt32 unrelated = 0;
        for ( size_t i=0; i<mails.size(); ++i )
        {
            if ( mails[ i ].data.HasSubstr( "X-Redux-Match:" ) >= 0 )
                continue;
            ++unrelated;
            const CORE::CString uid = CORE::UInt32ToString( mails[ i ].uid );
            // never named in a fetch or a store, never flagged
            for ( size_t c=0; c<commands.size(); ++c )
                ASSERT_FALSE( ( commands[ c ].HasSubstr( "FETCH " + uid + " " ) >= 0 || commands[ c ].HasSubstr( "STORE " + uid + " " ) >= 0 || commands[ c ].HasSubstr( "FETCH " + uid + ":" ) >= 0 ) );
            ASSERT_TRUE( mails[ i ].flags.empty() );
        }
        ASSERT_TRUE( 120 == unrelated );
        ASSERT_TRUE( CountCommands( commands, "FETCH" ) < 30 && CountCommands( commands, "X-Redux-Match" ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Bounces and automatic replies that carry the header are neither read nor answered, and mail of the player's own address is never processed" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Bounces and automatic replies that carry the header are neither read nor answered, and mail of the player's own address is never processed" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "transport-automatic" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_NEXT_ONLY, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        table.Settle( 300 );

        // a good turn of Bob, as it would be read, in the clothes of what must not be read
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 2, id, 0, "z1", package, preview ) );
        const GUCEF::vector< CORE::CString > toCarol = OneAddress( "carol@mail.test" );
        const CORE::CString good = table.TurnMailText( "bob@mail.test", toCarol, id, preview, package, MAIL::MAILTRANSFER_ATTACHMENT, "<good@crafted.test>" );
        ASSERT_FALSE( good.IsNULLOrEmpty() );
        const UInt32 acceptedBefore = table.world.GetAcceptedCount();
        const CORE::CString extras[] = { "Auto-Submitted: auto-replied\r\n", "Precedence: bulk\r\n", "X-Autoreply: yes\r\n", "Return-Path: <>\r\n", "X-Failed-Recipients: carol@mail.test\r\n" };
        for ( int i=0; i<5; ++i )
        {
            CORE::CString changed = extras[ i ] + good;
            changed = changed.ReplaceSubstr( "<good@crafted.test>", ( "<automatic" + CORE::UInt32ToString( i ) + "@crafted.test>" ).C_String() );
            ASSERT_TRUE( table.Inject( 2, changed ) );
        }
        CORE::CString bounce = good.ReplaceSubstr( "From: <bob@mail.test>", "From: <MAILER-DAEMON@mail.test>" );
        if ( bounce == good )
            bounce = "From: MAILER-DAEMON@mail.test\r\n" + good;
        bounce = bounce.ReplaceSubstr( "<good@crafted.test>", "<bounce@crafted.test>" );
        ASSERT_TRUE( table.Inject( 2, bounce ) );
        table.Settle( 1200 );
        ASSERT_TRUE( 1 == table.P( 2 ).provider->GetTrustedRevision( id ) && 0 == table.P( 2 ).recorder.refused.size() );
        // not answered: no acknowledgement went out for them
        ASSERT_TRUE( acceptedBefore == table.world.GetAcceptedCount() );
        // the same turn without the clothes is applied
        ASSERT_TRUE( table.Inject( 2, good ) );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) );

        // a good turn in a mail that comes from the player's own address
        MULTIPLAY::CMatchPackage next;
        MULTIPLAY::CMatchPreview nextPreview;
        ASSERT_TRUE( table.CraftTurn( table.P( 2 ).identity, 2, id, 0, "z2", next, nextPreview ) );
        ASSERT_TRUE( table.Inject( 0, table.TurnMailText( "alice@mail.test", OneAddress( "alice@mail.test" ), id, nextPreview, next, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        table.Settle( 800 );
        ASSERT_TRUE( 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail is marked read and may be copied to a folder, and never deleted" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail is marked read and may be copied to a folder, and never deleted" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        {
            CMailTable table;
            ASSERT_TRUE( table.Setup( 2, "transport-mark" ) );
            table.P( 1 ).box->imap.mailboxes.push_back( "GameMail" );
            table.P( 1 ).settings.processedFolder = "GameMail";
            ASSERT_TRUE( table.P( 1 ).provider->SetSettings( table.P( 1 ).settings ) && table.StartAll() );
            ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "friend@mail.test", "bob@mail.test", "Lunch?", "keep1" ) ) );
            MULTIPLAY::CMatchRecord record = table.MakeRecord( "Marked", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, record.info.matchId ) );
            table.Settle( 600 );
            GUCEF::vector< TESTFW::CTestImapServer::TMessage > mails = table.P( 1 ).box->imap.GetMessages();
            UInt32 seen = 0, ours = 0;
            for ( size_t i=0; i<mails.size(); ++i )
            {
                bool isSeen = false;
                for ( size_t f=0; f<mails[ i ].flags.size(); ++f )
                    isSeen = isSeen || mails[ i ].flags[ f ].Equals( "\\Seen", false );
                if ( mails[ i ].data.HasSubstr( "X-Redux-Match:" ) >= 0 )
                {
                    ++ours;
                    ASSERT_TRUE( isSeen );
                    ++seen;
                }
                else
                    ASSERT_FALSE( isSeen );
                // nothing is flagged for deletion
                for ( size_t f=0; f<mails[ i ].flags.size(); ++f )
                    ASSERT_FALSE( mails[ i ].flags[ f ].Equals( "\\Deleted", false ) );
            }
            ASSERT_TRUE( ours >= 1 && seen == ours );
            GUCEF::vector< CORE::CString > commands = table.P( 1 ).box->imap.GetCommands();
            ASSERT_TRUE( 0 == CountCommands( commands, "EXPUNGE" ) && 0 == CountCommands( commands, "Deleted" ) );
            // a copy of the processed mail is in the folder
            GUCEF::vector< TESTFW::CTestImapServer::TMessage > appended = table.P( 1 ).box->imap.GetAppended();
            ASSERT_TRUE( appended.size() >= 1 && appended[ 0 ].data.HasSubstr( "X-Redux-Match:" ) >= 0 );
        }
        {
            SOptions options;
            CMailTable table;
            ASSERT_TRUE( table.Setup( 2, "transport-no-mark", options ) );
            table.P( 1 ).settings.markProcessedSeen = false;
            ASSERT_TRUE( table.P( 1 ).provider->SetSettings( table.P( 1 ).settings ) && table.StartAll() );
            MULTIPLAY::CMatchRecord record = table.MakeRecord( "Unmarked", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, record.info.matchId ) );
            table.Settle( 400 );
            GUCEF::vector< TESTFW::CTestImapServer::TMessage > mails = table.P( 1 ).box->imap.GetMessages();
            ASSERT_TRUE( !mails.empty() && mails[ 0 ].flags.empty() );
            // and it is not read again for it
            const UInt32 handled = table.P( 1 ).provider->GetInbox().GetHandledCount();
            table.Advance( 5000, 600 );
            ASSERT_TRUE( handled == table.P( 1 ).provider->GetInbox().GetHandledCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail, LEAVE: the mail stays in the inbox, marked read, and nothing is copied" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail, LEAVE: the mail stays in the inbox, marked read, and nothing is copied" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SActionRun run;
        RunActionScenario( run, "action-leave", MAIL::MAILPROCESSED_LEAVE, true, true, true );
        ASSERT_TRUE( run.ran );
        ASSERT_TRUE( CountMatchMailsIn( run.inbox ) >= 3 && UnrelatedMailUntouched( run.inbox ) );
        for ( size_t i=0; i<run.inbox.size(); ++i )
            if ( IsMatchMail( run.inbox[ i ] ) )
                ASSERT_TRUE( HasFlagNamed( run.inbox[ i ], "\\Seen" ) && !HasFlagNamed( run.inbox[ i ], "\\Deleted" ) );
        ASSERT_TRUE( run.folder.empty() && run.appended.empty() );
        ASSERT_TRUE( 0 == CountCommands( run.commands, "COPY" ) && 0 == CountCommands( run.commands, "MOVE" ) && 0 == CountCommands( run.commands, "EXPUNGE" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail, MOVE with the MOVE extension: exactly the match mail moves in one command, other mail and flagged mail stay" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail, MOVE with the MOVE extension: exactly the match mail moves in one command, other mail and flagged mail stay" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SActionRun run;
        RunActionScenario( run, "action-move", MAIL::MAILPROCESSED_MOVE_TO_FOLDER, true, true, true );
        ASSERT_TRUE( run.ran );
        ASSERT_TRUE( 0 == CountMatchMailsIn( run.inbox ) );                              // all match mail left the inbox
        ASSERT_TRUE( UnrelatedMailUntouched( run.inbox ) );                              // the other three are as they were, the marked one included
        ASSERT_TRUE( run.folder.size() >= 3 && run.folder.size() == CountMatchMailsIn( run.folder ) );
        ASSERT_TRUE( CountCommands( run.commands, "UID MOVE" ) == (UInt32) run.folder.size() );
        ASSERT_TRUE( 0 == CountCommands( run.commands, "EXPUNGE" ) && 0 == CountCommands( run.commands, "+FLAGS (\\Deleted)" ) && 0 == CountCommands( run.commands, "COPY" ) );
        ASSERT_TRUE( run.problems.empty() );
        for ( size_t i=0; i<run.folder.size(); ++i )
            ASSERT_TRUE( HasFlagNamed( run.folder[ i ], "\\Seen" ) );                    // read mail arrives read
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail, MOVE without the MOVE extension: copy, mark deleted and UID EXPUNGE of that UID, never a plain EXPUNGE, other mail marked deleted survives" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail, MOVE without the MOVE extension: copy, mark deleted and UID EXPUNGE of that UID, never a plain EXPUNGE, other mail marked deleted survives" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SActionRun run;
        RunActionScenario( run, "action-move-uidplus", MAIL::MAILPROCESSED_MOVE_TO_FOLDER, false, true, true );
        ASSERT_TRUE( run.ran );
        ASSERT_TRUE( 0 == CountMatchMailsIn( run.inbox ) && UnrelatedMailUntouched( run.inbox ) );
        ASSERT_TRUE( run.folder.size() >= 3 && run.folder.size() == CountMatchMailsIn( run.folder ) );
        ASSERT_TRUE( 0 == CountCommands( run.commands, "MOVE" ) );
        ASSERT_TRUE( CountCommands( run.commands, "UID COPY" ) == (UInt32) run.folder.size() );
        ASSERT_TRUE( CountCommands( run.commands, "UID EXPUNGE" ) == (UInt32) run.folder.size() );
        ASSERT_TRUE( CountCommands( run.commands, "+FLAGS.SILENT (\\Deleted)" ) == (UInt32) run.folder.size() );
        for ( size_t i=0; i<run.commands.size(); ++i )                                   // a plain EXPUNGE would remove the letter the player marked
            ASSERT_FALSE( run.commands[ i ].HasSubstr( "EXPUNGE" ) >= 0 && run.commands[ i ].HasSubstr( "UID EXPUNGE" ) < 0 );
        ASSERT_TRUE( run.problems.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail, MOVE on a server with neither MOVE nor UIDPLUS: the mail is copied and stays, and the player is told once, in plain words" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail, MOVE on a server with neither MOVE nor UIDPLUS: the mail is copied and stays, and the player is told once, in plain words" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SActionRun run;
        RunActionScenario( run, "action-move-none", MAIL::MAILPROCESSED_MOVE_TO_FOLDER, false, false, true );
        ASSERT_TRUE( run.ran );
        ASSERT_TRUE( CountMatchMailsIn( run.inbox ) >= 3 && UnrelatedMailUntouched( run.inbox ) );   // the mail stays where it was
        ASSERT_TRUE( run.folder.size() == CountMatchMailsIn( run.inbox ) && run.folder.size() == CountMatchMailsIn( run.folder ) );
        ASSERT_TRUE( 0 == CountCommands( run.commands, "EXPUNGE" ) && 0 == CountCommands( run.commands, "Deleted" ) && 0 == CountCommands( run.commands, "MOVE" ) );
        UInt32 notices = 0;
        for ( size_t i=0; i<run.problems.size(); ++i )
            if ( run.problems[ i ].code == "MOVE_NOT_POSSIBLE" )
            {
                ++notices;
                ASSERT_TRUE( run.problems[ i ].description.HasSubstr( "copied" ) >= 0 && run.problems[ i ].description.HasSubstr( "move" ) >= 0 );
                ASSERT_TRUE( run.problems[ i ].description.HasSubstr( "@" ) < 0 );
            }
        ASSERT_TRUE( 1 == notices );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail, a move that fails (the folder does not exist): the mail stays where it is, is not flagged deleted, is not read again, and the player is told" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail, a move that fails (the folder does not exist): the mail stays where it is, is not flagged deleted, is not read again, and the player is told" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        for ( int variant=0; variant<2; ++variant )
        {
            SActionRun run;
            RunActionScenario( run, variant ? "action-move-fail-a" : "action-move-fail-b", MAIL::MAILPROCESSED_MOVE_TO_FOLDER, 0 == variant, true, false );
            ASSERT_TRUE( run.ran );                                                      // the match went on: a failed move is not a failed mailbox
            ASSERT_TRUE( CountMatchMailsIn( run.inbox ) >= 3 && UnrelatedMailUntouched( run.inbox ) );
            for ( size_t i=0; i<run.inbox.size(); ++i )
                ASSERT_FALSE( HasFlagNamed( run.inbox[ i ], "\\Deleted" ) && IsMatchMail( run.inbox[ i ] ) );
            ASSERT_TRUE( run.folder.empty() && 0 == CountCommands( run.commands, "EXPUNGE" ) );
            UInt32 notices = 0;
            for ( size_t i=0; i<run.problems.size(); ++i )
                if ( run.problems[ i ].code == "MOVE_FAILED" )
                    ++notices;
            ASSERT_TRUE( notices >= 1 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Processed mail, COPY_TO_FOLDER: a copy is put in the folder and the original stays in the inbox, not deleted" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Processed mail, COPY_TO_FOLDER: a copy is put in the folder and the original stays in the inbox, not deleted" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SActionRun run;
        RunActionScenario( run, "action-copy", MAIL::MAILPROCESSED_COPY_TO_FOLDER, true, true, true );
        ASSERT_TRUE( run.ran );
        ASSERT_TRUE( CountMatchMailsIn( run.inbox ) >= 3 && UnrelatedMailUntouched( run.inbox ) );
        ASSERT_TRUE( run.folder.size() == CountMatchMailsIn( run.inbox ) && run.folder.size() == CountMatchMailsIn( run.folder ) );
        ASSERT_TRUE( 0 == CountCommands( run.commands, "EXPUNGE" ) && 0 == CountCommands( run.commands, "MOVE" ) && 0 == CountCommands( run.commands, "Deleted" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "POP3: with pop3DeleteAfterProcessing the mails of matches that were processed are deleted at the end of the session, other mail never, and by default nothing" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "POP3: with pop3DeleteAfterProcessing the mails of matches that were processed are deleted at the end of the session, other mail never, and by default nothing" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions options;
        options.useImap = false;
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-pop3-delete", options ) );
        table.P( 1 ).settings.pop3DeleteAfterProcessing = true;
        ASSERT_TRUE( table.P( 1 ).provider->SetSettings( table.P( 1 ).settings ) && table.StartAll() );
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "friend@mail.test", "bob@mail.test", "Lunch?", "l1" ) ) );
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", PlainMail( "friend@mail.test", "bob@mail.test", "[Redux 12345678 r1 t1] looks like it but is not", "l2" ) ) );
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Pop delete", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) );
        for ( int i=0; i<12; ++i )
            table.Advance( 600, 150 );
        MULTIPLAY::CMatchInfo known;
        ASSERT_TRUE( table.P( 1 ).provider->GetMatchInfo( record.info.matchId, known ) );
        // the invitation was number 3, it was read and deleted; the letters (1 and 2) were never read, never deleted
        GUCEF::vector< CORE::CString > commands = table.P( 1 ).box->pop3.GetCommands();
        ASSERT_TRUE( 1 == CountCommands( commands, "RETR 3" ) && 0 == CountCommands( commands, "RETR 1" ) && 0 == CountCommands( commands, "RETR 2" ) );
        ASSERT_TRUE( 1 == CountCommands( commands, "DELE 3" ) && 0 == CountCommands( commands, "DELE 1" ) && 0 == CountCommands( commands, "DELE 2" ) );
        ASSERT_TRUE( 1 == table.P( 1 ).box->pop3.GetCommittedDeletes() );
        // and the game goes on
        ASSERT_TRUE( table.Respond( 1, record.info.matchId, true, result ) );
        ASSERT_TRUE( table.PlayTurn( 1, record.info.matchId, 0, "p1", result ) );
        bool second = false;
        for ( int i=0; i<40 && !second; ++i )
        {
            table.Advance( 600, 150 );
            second = table.P( 0 ).provider->GetTrustedRevision( record.info.matchId ) >= 2;
        }
        ASSERT_TRUE( second );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Seen set: the message ids that are remembered are bounded, and a mail is read once" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Seen set: the message ids that are remembered are bounded, and a mail is read once" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions options;
        options.seenSetSize = 10;
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "transport-seen", options ) && table.StartAll() );
        const CORE::CString matchId = MULTIPLAY::CMatchId::Generate();
        for ( int i=0; i<25; ++i )
            ASSERT_TRUE( table.world.DeliverTo( "alice@mail.test", AckMail( matchId, 1, ( "ack" + CORE::UInt32ToString( i ) ).C_String() ) ) );
        bool read = false;
        PUMP_WAIT( read, table.P( 0 ).provider->GetInbox().GetHandledCount() >= 25, 15000 );
        ASSERT_TRUE( read );
        ASSERT_TRUE( table.P( 0 ).provider->GetInbox().GetSeenCount() <= 10 && table.P( 0 ).provider->GetInbox().GetSeenCount() >= 5 );
        // one of the newest again: known, so not handled again
        const UInt32 handled = table.P( 0 ).provider->GetInbox().GetHandledCount();
        ASSERT_TRUE( table.world.DeliverTo( "alice@mail.test", AckMail( matchId, 1, "ack24" ) ) );
        table.Settle( 800 );
        ASSERT_TRUE( handled == table.P( 0 ).provider->GetInbox().GetHandledCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mailbox connection: a dropped connection is made again, a refused login is reported and the player's secrets are in no log" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Mailbox connection: a dropped connection is made again, a refused login is reported and the player's secrets are in no log" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        {
            SOptions options;
            options.autoClockMs = 2;
            CMailTable table;
            ASSERT_TRUE( table.Setup( 2, "transport-drop", options ) );
            // the connection of Bob is ended by the server after 16 commands, which is a few mails
            table.P( 1 ).box->imap.dropAfterCommands = 16;
            ASSERT_TRUE( table.StartAll() );
            const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
            if ( id.IsNULLOrEmpty() )
                for ( UInt32 i=0; i<2; ++i )
                    MAIL_TEST_NOTE( "drop: " << table.Diagnose( i ).C_String() );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.PlayTurn( 1, id, 0, "d1", result ) && table.WaitTrusted( 0, id, 2 ) );
            ASSERT_TRUE( table.PlayTurn( 0, id, 1, "d2", result ) && table.WaitTrusted( 1, id, 3, 15000 ) );
            bool wasOffline = false, wasOnline = false;
            for ( size_t i=0; i<table.P( 1 ).recorder.states.size(); ++i )
            {
                if ( !table.P( 1 ).recorder.states[ i ] ) wasOffline = true;
                else if ( wasOffline ) wasOnline = true;
            }
            ASSERT_TRUE( wasOffline && wasOnline && table.P( 1 ).box->imap.GetAcceptedConnections() >= 2 );
        }
        {
            // the secrets: a password from a credential provider, one that is wrong, the contents and the addresses
            TESTFW::CLogTextCapture capture;
            SOptions options;
            options.credentialsFromProvider = true;
            CMailTable table;
            ASSERT_TRUE( table.Setup( 3, "transport-secrets", options ) );
            // Carol's IMAP login is refused: her credential provider has a wrong password
            table.P( 2 ).credentials.SetCredential( COM::MAILCRED_PASSWORD, table.P( 2 ).box->user, "WRONG-PW-5521" );
            ASSERT_TRUE( table.StartAll() );
            const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
            // two of three can play: the match starts without Carol, who never answers
            ASSERT_TRUE( id.IsNULLOrEmpty() );
            bool refused = false;
            PUMP_WAIT( refused, !table.P( 2 ).recorder.errors.empty() || ( !table.P( 2 ).recorder.states.empty() && !table.P( 2 ).recorder.states.back() ), 8000 );
            ASSERT_TRUE( refused );
            ASSERT_FALSE( table.P( 2 ).provider->GetInbox().IsOnline() );
            MULTIPLAY::CMatchOperationResult result;
            MULTIPLAY::CMatchInfo info;
            table.Settle( 800 );
            for ( size_t i=0; i<table.P( 2 ).recorder.errors.size(); ++i )
                ASSERT_TRUE( table.P( 2 ).recorder.errors[ i ].HasSubstr( "WRONG-PW-5521" ) < 0 );

            const CORE::CString text = capture.GetText();
            ASSERT_TRUE( text.Length() > 100 );
            ASSERT_TRUE( text.HasSubstr( "WRONG-PW-5521" ) < 0 );
            ASSERT_TRUE( text.HasSubstr( "Zq81" ) < 0 );
            ASSERT_TRUE( text.HasSubstr( "@mail.test" ) < 0 );
            ASSERT_TRUE( text.HasSubstr( "game|setup" ) < 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
