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

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#include "TestMailFixture.h"
#include "TestMailLifecycle.h"

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

/** The mails of a mailbox that are of the kind, parsed */
GUCEF::vector< COM::CMailMessage >
MailsOfKind( CMailTable& table, UInt32 player, const char* kind )
{GUCEF_TRACE;

    GUCEF::vector< COM::CMailMessage > found;
    GUCEF::vector< TESTFW::CTestImapServer::TMessage > messages = table.players[ player ]->box->imap.GetMessages();
    COM::CMimeParser parser;
    for ( size_t i=0; i<messages.size(); ++i )
    {
        COM::CMailMessage message;
        if ( parser.ParseMessage( messages[ i ].data.C_String(), messages[ i ].data.ByteSize() - 1, message ) && message.headers.Get( "X-Redux-Kind" ) == kind )
            found.push_back( message );
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
HasAddress( const COM::CMailAddress::TMailAddressVector& list, const CORE::CString& address )
{GUCEF_TRACE;

    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ].GetAddress().Lowercase() == address.Lowercase() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
RevisionOf( const COM::CMailMessage& message )
{GUCEF_TRACE;

    return message.headers.Get( "X-Redux-Revision" );
}

/*-------------------------------------------------------------------------*/

/** Plays turns in the round robin Alice(0), Bob(1), Carol(2): the next one after the player plays; true when every provider has the revision */
bool
PlayRound( CMailTable& table, const CORE::CString& matchId, UInt32 player, UInt32 playerCount, const char* move, bool last )
{GUCEF_TRACE;

    MULTIPLAY::CMatchOperationResult result;
    const int next = last ? -1 : (int) ( ( player + 1 ) % playerCount );
    if ( !table.PlayTurn( player, matchId, next, move, result, last ? MULTIPLAY::MATCHSTATUS_FINISHED : MULTIPLAY::MATCHSTATUS_ACTIVE ) || MULTIPLAY::MATCHOPERATION_OK != result.status )
    {
        // what the provider answered is part of the failure
        MAIL_TEST_NOTE( "PlayRound " << move << ": status " << MULTIPLAY::MatchOperationStatusToString( result.status ) << ", reason " << result.reason.C_String() << ", trusted " << table.P( player ).provider->GetTrustedRevision( matchId ) );
        return false;
    }
    if ( !table.WaitAllTrusted( matchId, result.preview.revision ) )
    {
        for ( UInt32 i=0; i<table.players.size(); ++i )
            MAIL_TEST_NOTE( "PlayRound " << move << " wanted revision " << result.preview.revision << ", trusted " << table.P( i ).provider->GetTrustedRevision( matchId ) << ": " << table.Diagnose( i ).C_String() );
        return false;
    }
    return true;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailLifecycleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: the life of a match by mail" );

    GUCEF_TESTFW_TESTCASE( "Provider: the plugin registers the provider \"mail\" with the capabilities of a provider without a clock, a push or a live game" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Provider: the plugin registers the provider \"mail\" with the capabilities of a provider without a clock, a push or a live game" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 codec" );
        MULTIPLAY::CIMatchProviderPtr made = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "mail" );
        ASSERT_FALSE( made.IsNULL() );
        ASSERT_TRUE( CORE::CString( "mail" ) == made->GetProviderName() );
        const UInt32 expected = MULTIPLAY::MATCHCAPABILITY_NUDGE | MULTIPLAY::MATCHCAPABILITY_RESIGN | MULTIPLAY::MATCHCAPABILITY_REMATCH;
        ASSERT_TRUE( expected == made->GetCapabilities() );
        ASSERT_FALSE( made->HasCapability( MULTIPLAY::MATCHCAPABILITY_SERVER_CLOCK ) || made->HasCapability( MULTIPLAY::MATCHCAPABILITY_PUSH ) || made->HasCapability( MULTIPLAY::MATCHCAPABILITY_LIVE ) );
        ASSERT_FALSE( made->IsStarted() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Provider: it starts only with a usable account and an identity, and stops cleanly" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Provider: it starts only with a usable account and an identity, and stops cleanly" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "provider-start" ) );
        MAIL::CMailMatchProvider* provider = table.P( 0 ).provider;
        ASSERT_FALSE( provider->IsStarted() );
        ASSERT_TRUE( provider->Start() && provider->IsStarted() );
        ASSERT_TRUE( provider->Start() );
        provider->Stop();
        ASSERT_FALSE( provider->IsStarted() );
        ASSERT_TRUE( provider->Start() );

        // no account
        MAIL::CMailMatchProvider bare;
        ASSERT_FALSE( bare.Start() || bare.IsStarted() );
        // no identity
        MAIL::CMailMatchProvider anonymous;
        ASSERT_TRUE( anonymous.SetSettings( table.P( 1 ).settings ) );
        ASSERT_FALSE( anonymous.Start() );
        anonymous.SetIdentity( table.P( 1 ).identity.GetPublicIdentity() );
        ASSERT_FALSE( anonymous.Start() );
        // settings cannot change under a running provider
        ASSERT_FALSE( provider->SetSettings( table.P( 0 ).settings ) );
        provider->Stop();
        ASSERT_TRUE( provider->SetSettings( table.P( 0 ).settings ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match life: invitation with the readable text and the link, acceptance by signed mail, and the match is active when all accepted" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match life: invitation with the readable text and the link, acceptance by signed mail, and the match is active when all accepted" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-invite" ) && table.StartAll() );

        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Friday game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        const CORE::CString id = record.info.matchId;
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        // the creator has the match at once, before any mail was delivered
        ASSERT_TRUE( table.P( 0 ).recorder.CountAdded( id ) == 1 && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );

        // what the invitee sees: forming, invited, the first turn is theirs to play once everybody accepted
        CProviderRecorder::SUpdate seen;
        ASSERT_TRUE( table.P( 1 ).recorder.GetLast( id, seen ) && seen.hasInfo && 1 == seen.revision && MULTIPLAY::MATCHSTATUS_FORMING == seen.status );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, id, 1 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, id, 0 ) );
        ASSERT_TRUE( seen.info.creatorUserId == table.P( 0 ).identity.userId && "Friday game" == seen.info.title && 1 == table.P( 1 ).provider->GetTrustedRevision( id ) );
        ASSERT_TRUE( seen.preview.IsCurrent( table.P( 1 ).identity.userId ) );

        // the mail: subject format, readable text with the link, headers
        GUCEF::vector< COM::CMailMessage > invites = MailsOfKind( table, 1, "INVITE" );
        ASSERT_TRUE( 1 == invites.size() );
        ASSERT_TRUE( invites[ 0 ].subject == "[Redux " + id.SubstrToIndex( 8, true ) + " r1 t1] Friday game" || invites[ 0 ].subject.StartsWith( "[Redux " + id.SubstrToIndex( 8, true ) + " r1 t" ) );
        ASSERT_TRUE( invites[ 0 ].subject.HasSubstr( "] Friday game" ) >= 0 && invites[ 0 ].headers.Get( "X-Redux-Match" ) == id && invites[ 0 ].headers.Get( "X-Redux-Protocol" ) == "1" );
        ASSERT_TRUE( invites[ 0 ].textBody.HasSubstr( "https://games.example.invalid/match/" + id ) >= 0 && invites[ 0 ].textBody.HasSubstr( "Friday game" ) >= 0 && invites[ 0 ].textBody.HasSubstr( "Alice" ) >= 0 );
        ASSERT_TRUE( HasAddress( invites[ 0 ].to, "bob@mail.test" ) && invites[ 0 ].to.size() == 1 );
        ASSERT_TRUE( invites[ 0 ].attachments.size() >= 1 );
        for ( size_t i=0; i<invites[ 0 ].attachments.size(); ++i )
            ASSERT_TRUE( invites[ 0 ].attachments[ i ].fileName.StartsWith( "redux-" + id.SubstrToIndex( 8, true ) + "-" ) );

        // acceptances by signed mail to the creator and, so that everybody sees them, to the others
        ASSERT_TRUE( table.Respond( 1, id, true, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        bool bobSeenByAll = false;
        PUMP_WAIT( bobSeenByAll, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 1 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 2, id, 1 ), 8000 );
        if ( !bobSeenByAll )
            for ( UInt32 i=0; i<3; ++i )
                MAIL_TEST_NOTE( "bob accepted: " << (Int32) table.StatusOf( i, id, 1 ) << " " << table.Diagnose( i ).C_String() );
        ASSERT_TRUE( bobSeenByAll );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_FORMING == table.MatchStatusOf( 0, id ) && MULTIPLAY::MATCHSTATUS_FORMING == table.MatchStatusOf( 1, id ) );
        ASSERT_TRUE( table.Respond( 2, id, true, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        for ( UInt32 i=0; i<3; ++i )
        {
            bool active = false;
            PUMP_WAIT( active, MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( i, id ), 8000 );
            ASSERT_TRUE( active );
            for ( UInt32 j=0; j<3; ++j )
                ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( i, id, j ) );
        }
        ASSERT_TRUE( table.P( 1 ).recorder.updated.size() >= 1 );
        GUCEF::vector< COM::CMailMessage > accepts = MailsOfKind( table, 0, "ACCEPT" );
        ASSERT_TRUE( 2 == accepts.size() );
        ASSERT_TRUE( 1 == MailsOfKind( table, 2, "ACCEPT" ).size() && 1 == MailsOfKind( table, 1, "ACCEPT" ).size() );
        // the acceptance is a signed block, not a promise in prose: it travels as data
        ASSERT_TRUE( 1 == accepts[ 0 ].attachments.size() );
        // an acceptance is only possible for a match the player was invited to and by the player themselves
        ASSERT_FALSE( table.Respond( 1, MULTIPLAY::CMatchId::Generate(), true, result ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match life: a decline is known to everybody, the turn cannot be played before all answered, and the first turn starts the match without the decliner" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match life: a decline is known to everybody, the turn cannot be played before all answered, and the first turn starts the match without the decliner" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-decline" ) && table.StartAll() );
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Declined", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        const CORE::CString id = record.info.matchId;
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );

        ASSERT_TRUE( table.Respond( 1, id, true, result ) );
        bool seen = false;
        PUMP_WAIT( seen, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, id, 1 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 1 ), 8000 );
        if ( !seen ) { MAIL_TEST_NOTE( table.Diagnose( 0 ).C_String() ); MAIL_TEST_NOTE( table.Diagnose( 1 ).C_String() ); }
        ASSERT_TRUE( seen );
        // Carol has not answered: Bob may not play yet
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "early", result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status && !result.reason.IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == table.P( 1 ).provider->GetTrustedRevision( id ) );

        ASSERT_TRUE( table.Respond( 2, id, false, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        PUMP_WAIT( seen, MULTIPLAY::MATCHPARTICIPANT_DECLINED == table.StatusOf( 0, id, 2 ) && MULTIPLAY::MATCHPARTICIPANT_DECLINED == table.StatusOf( 1, id, 2 ), 8000 );
        ASSERT_TRUE( seen );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_FORMING == table.MatchStatusOf( 0, id ) );

        // everybody answered: the first turn is the start without the one who declined
        ASSERT_TRUE( table.PlayTurn( 1, id, 0, "first", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 2 ) );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( 0, id ) && MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( 1, id ) );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_DECLINED == table.StatusOf( 0, id, 2 ) );
        // and the one who declined is not mailed the game
        table.Settle( 500 );
        ASSERT_TRUE( 0 == table.CountMatchMails( 2, "TURN" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match life: turns by attachment to everybody, to the end; every participant holds and verified every revision, and the mails are threaded" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match life: turns by attachment to everybody, to the end; every participant holds and verified every revision, and the mails are threaded" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-all-attachment" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        ASSERT_TRUE( PlayRound( table, id, 1, 3, "b1", false ) );       // r2
        ASSERT_TRUE( PlayRound( table, id, 2, 3, "c1", false ) );       // r3
        ASSERT_TRUE( PlayRound( table, id, 0, 3, "a1", false ) );       // r4
        ASSERT_TRUE( PlayRound( table, id, 1, 3, "b2", false ) );       // r5
        ASSERT_TRUE( PlayRound( table, id, 2, 3, "c2", true ) );        // r6, the last

        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( 6 == table.P( i ).provider->GetTrustedRevision( id ) );
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_FINISHED == table.MatchStatusOf( i, id ) );
            for ( UInt32 rev=1; rev<=6; ++rev )
            {
                MULTIPLAY::CMatchPreview preview;
                MULTIPLAY::CMatchPackage package;
                ASSERT_TRUE( table.P( i ).provider->GetStoredTurn( id, rev, preview, package ) && preview.revision == rev && preview.VerifySignature() && package.IsConsistent() );
            }
            // the game's replay ran for the turns of the others
            if ( i != 1 )
                ASSERT_TRUE( table.P( i ).verifier.calls >= 2 );
        }
        // the newest package carries all moves
        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 6, preview, package ) );
        ASSERT_TRUE( CGameVerifier::Text( package ) == "game|setup|start|b1|c1|a1|b2|c2" && MULTIPLAY::MATCHSTATUS_FINISHED == preview.status );

        // one mail per turn: To the next holder, Cc the others; the last one is a FINISH
        GUCEF::vector< COM::CMailMessage > turnsAtAlice = MailsOfKind( table, 0, "TURN" );
        ASSERT_TRUE( 3 == turnsAtAlice.size() );
        for ( size_t i=0; i<turnsAtAlice.size(); ++i )
        {
            if ( RevisionOf( turnsAtAlice[ i ] ) == "2" )
            {
                ASSERT_TRUE( 1 == turnsAtAlice[ i ].to.size() && HasAddress( turnsAtAlice[ i ].to, "carol@mail.test" ) );
                ASSERT_TRUE( 1 == turnsAtAlice[ i ].cc.size() && HasAddress( turnsAtAlice[ i ].cc, "alice@mail.test" ) );
                ASSERT_TRUE( turnsAtAlice[ i ].subject.StartsWith( "[Redux " + id.SubstrToIndex( 8, true ) + " r2 t2] Friday game" ) );
                ASSERT_TRUE( 1 == turnsAtAlice[ i ].attachments.size() && turnsAtAlice[ i ].textBody.HasSubstr( "BEGIN" ) < 0 );
                ASSERT_TRUE( turnsAtAlice[ i ].textBody.HasSubstr( "Bob" ) >= 0 && turnsAtAlice[ i ].textBody.HasSubstr( "Carol" ) >= 0 );
            }
        }
        GUCEF::vector< COM::CMailMessage > finishAtAlice = MailsOfKind( table, 0, "FINISH" );
        ASSERT_TRUE( 1 == finishAtAlice.size() && HasAddress( finishAtAlice[ 0 ].to, "alice@mail.test" ) && RevisionOf( finishAtAlice[ 0 ] ) == "6" );

        // threading: the mail of Carol for r3 answers the mail of Bob for r2, which she had received
        COM::CMailMessage r2, r3;
        GUCEF::vector< COM::CMailMessage > turnsAtCarol = MailsOfKind( table, 2, "TURN" );
        GUCEF::vector< COM::CMailMessage > turnsAtBob = MailsOfKind( table, 1, "TURN" );
        for ( size_t i=0; i<turnsAtCarol.size(); ++i )
            if ( RevisionOf( turnsAtCarol[ i ] ) == "2" ) r2 = turnsAtCarol[ i ];
        for ( size_t i=0; i<turnsAtBob.size(); ++i )
            if ( RevisionOf( turnsAtBob[ i ] ) == "3" ) r3 = turnsAtBob[ i ];
        ASSERT_TRUE( !r2.messageId.IsNULLOrEmpty() && !r3.messageId.IsNULLOrEmpty() && r3.messageId != r2.messageId );
        ASSERT_TRUE( 1 == r3.inReplyTo.size() && r3.inReplyTo[ 0 ] == r2.messageId );
        bool referenced = false;
        for ( size_t i=0; i<r3.references.size(); ++i )
            referenced = referenced || r3.references[ i ] == r2.messageId;
        ASSERT_TRUE( referenced );

        // everything that was sent shows as delivered: the receivers acknowledged
        const UInt32 sender[ 5 ] = { 1, 2, 0, 1, 2 };
        bool delivered = false;
        PUMP_WAIT( delivered, MAIL::MAILDELIVERY_DELIVERED == table.P( 1 ).provider->GetDeliveryState( id, 2 ) && MAIL::MAILDELIVERY_DELIVERED == table.P( 2 ).provider->GetDeliveryState( id, 3 ) &&
                              MAIL::MAILDELIVERY_DELIVERED == table.P( 0 ).provider->GetDeliveryState( id, 4 ) && MAIL::MAILDELIVERY_DELIVERED == table.P( 1 ).provider->GetDeliveryState( id, 5 ) &&
                              MAIL::MAILDELIVERY_DELIVERED == table.P( 2 ).provider->GetDeliveryState( id, 6 ), 5000 );
        for ( UInt32 rev=2; rev<=6; ++rev )
        {
            UInt32 attempts = 0;
            const MAIL::TMailDeliveryState state = table.P( sender[ rev - 2 ] ).provider->GetDeliveryState( id, rev, &attempts );
            if ( MAIL::MAILDELIVERY_DELIVERED != state )
                MAIL_TEST_NOTE( "revision " << rev << " of player " << sender[ rev - 2 ] << ": " << MAIL::MailDeliveryStateToString( state ) << ", attempts " << attempts << ", unconfirmed " << table.P( sender[ rev - 2 ] ).provider->GetOutbox().GetUnconfirmedCount() );
            ASSERT_TRUE( MAIL::MAILDELIVERY_DELIVERED == state );
        }
        ASSERT_TRUE( delivered );
        ASSERT_TRUE( table.P( 1 ).recorder.HasDelivery( id, 2, MAIL::MAILDELIVERY_SENT ) && table.P( 1 ).recorder.HasDelivery( id, 2, MAIL::MAILDELIVERY_DELIVERED ) );
        ASSERT_TRUE( 0 == table.P( 1 ).provider->GetOutbox().GetUnconfirmedCount() || table.P( 1 ).provider->GetOutbox().GetUnconfirmedCount() <= 1 );
        // the tiny acknowledgements: they carry no attachment
        GUCEF::vector< COM::CMailMessage > acks = MailsOfKind( table, 1, "ACK" );
        ASSERT_TRUE( acks.size() >= 2 );
        for ( size_t i=0; i<acks.size(); ++i )
            ASSERT_TRUE( acks[ i ].attachments.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match life: turns as text in the body to the next player only; the player who missed one asks the table for it and catches up" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match life: turns as text in the body to the next player only; the player who missed one asks the table for it and catches up" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-next-text" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_BODY_TEXT, MAIL::MAILDISTRIBUTION_NEXT_ONLY, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        const UInt32 turnsBefore = table.CountMatchMails( 0, "TURN" );

        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) );
        table.Settle( 300 );
        // Alice was not mailed the second revision
        ASSERT_TRUE( turnsBefore == table.CountMatchMails( 0, "TURN" ) && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );

        ASSERT_TRUE( table.PlayTurn( 2, id, 0, "c1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        // Alice gets the third revision, sees that the second is missing, asks, and is served by the table
        ASSERT_TRUE( table.WaitTrusted( 0, id, 3 ) );
        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 2, preview, package ) && preview.VerifyWithKey( table.P( 1 ).identity.publicKey ) );
        ASSERT_TRUE( CGameVerifier::Text( package ) == "game|setup|start|b1" );
        ASSERT_TRUE( table.CountMatchMails( 1, "RESEND" ) >= 1 && table.CountMatchMails( 2, "RESEND" ) >= 1 );
        // the text mails have no attachment and the data between the marker lines
        GUCEF::vector< COM::CMailMessage > turnsAtAlice = MailsOfKind( table, 0, "TURN" );
        ASSERT_TRUE( turnsAtAlice.size() >= 2 );
        for ( size_t i=0; i<turnsAtAlice.size(); ++i )
            ASSERT_TRUE( turnsAtAlice[ i ].attachments.empty() && turnsAtAlice[ i ].textBody.HasSubstr( "-----BEGIN MATCH TURN-----" ) >= 0 );
        // asked once for the revision, not over and over
        table.Settle( 500 );
        ASSERT_TRUE( 1 == table.CountMatchMails( 1, "RESEND" ) );
        // the newest package is the third and the chain went on from the second
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 3, preview, package ) && CGameVerifier::Text( package ) == "game|setup|start|b1|c1" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match life: the receiver reads both forms in any match, a turn on a pasted text and a turn in an attachment of a match that sends text" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match life: the receiver reads both forms in any match, a turn on a pasted text and a turn in an attachment of a match that sends text" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-both-forms" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_NEXT_ONLY, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 2, id, 2 ) );

        // Alice never received it; Bob gives her the text to paste into a message of her own
        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        ASSERT_TRUE( table.P( 1 ).provider->GetStoredTurn( id, 2, preview, package ) );
        CORE::CString text, reason;
        ASSERT_TRUE( MAIL::CMailMatchProvider::ExportToText( preview, package, text, reason ) );
        ASSERT_TRUE( text.HasSubstr( "-----BEGIN" ) >= 0 && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        ASSERT_TRUE( table.P( 0 ).provider->ImportText( "Hi Alice, here is my move.\r\n\r\n" + text + "\r\nBob\r\n", reason ) );
        ASSERT_TRUE( 2 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        // the same text again changes nothing
        ASSERT_TRUE( table.P( 0 ).provider->ImportText( text, reason ) );
        ASSERT_TRUE( 2 == table.P( 0 ).provider->GetTrustedRevision( id ) && 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Submit: the answers follow the revision the caller expected, a repeat is safe, a wrong turn is refused and the future ends when the mail is in the outbox" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Submit: the answers follow the revision the caller expected, a repeat is safe, a wrong turn is refused and the future ends when the mail is in the outbox" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-submit" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        MULTIPLAY::CMatchPackage package, other;
        MULTIPLAY::CMatchPreview preview, otherPreview;
        ASSERT_TRUE( table.MakeTurn( 1, id, 2, "b1", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) && table.MakeTurn( 1, id, 2, "b1-different", MULTIPLAY::MATCHSTATUS_ACTIVE, other, otherPreview ) );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 1 ).provider->Submit( id, 1, preview, package, future ) );
        // the call is answered when the turn is signed and in the outbox, not when the mail has arrived
        ASSERT_TRUE( table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status && result.hasPreview && 2 == result.preview.revision );
        ASSERT_TRUE( 2 == table.P( 1 ).provider->GetTrustedRevision( id ) );
        ASSERT_TRUE( MAIL::MAILDELIVERY_NONE != table.P( 1 ).provider->GetDeliveryState( id, 2 ) );
        ASSERT_TRUE( 1 == table.P( 1 ).recorder.submitResults.size() && MULTIPLAY::MATCHOPERATION_OK == table.P( 1 ).recorder.submitResults[ 0 ].status );

        // the same package again: OK again, no second mail
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) );
        table.Settle( 300 );
        const UInt32 mailsAtCarol = table.CountMatchMails( 2, "TURN" );
        ASSERT_TRUE( table.P( 1 ).provider->Submit( id, 1, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        table.Settle( 300 );
        ASSERT_TRUE( mailsAtCarol == table.CountMatchMails( 2, "TURN" ) );
        // another package for a revision that is held: the caller is behind
        ASSERT_TRUE( table.P( 1 ).provider->Submit( id, 1, otherPreview, other, future ) && table.Await( future, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_STALE == result.status && result.hasPreview && 2 == result.preview.revision );

        // not the turn of the player
        ASSERT_TRUE( table.PlayTurn( 0, id, 1, "a-too-early", result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status && !result.reason.IsNULLOrEmpty() && 2 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        // an expected revision from the future, a preview of another revision than the package, an unknown match
        MULTIPLAY::CMatchPackage next;
        MULTIPLAY::CMatchPreview nextPreview;
        ASSERT_TRUE( table.MakeTurn( 2, id, 0, "c1", MULTIPLAY::MATCHSTATUS_ACTIVE, next, nextPreview ) );
        ASSERT_TRUE( table.P( 2 ).provider->Submit( id, 7, nextPreview, next, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 2 ).provider->Submit( id, 2, preview, next, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 2 ).provider->Submit( MULTIPLAY::CMatchId::Generate(), 2, nextPreview, next, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_NOT_FOUND == result.status );
        // a package that does not follow the newest one
        MULTIPLAY::CMatchPackage broken = next;
        broken.baseSha256 = "00";
        ASSERT_TRUE( table.P( 2 ).provider->Submit( id, 2, nextPreview, broken, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        // nothing of that changed the match
        ASSERT_TRUE( 2 == table.P( 2 ).provider->GetTrustedRevision( id ) );
        // and the right turn still works
        ASSERT_TRUE( table.P( 2 ).provider->Submit( id, 2, nextPreview, next, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Submit: the game's check is required to apply a turn of another player, unless the application says it has none" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Submit: the game's check is required to apply a turn of another player, unless the application says it has none" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        {
            SOptions options;
            options.installVerifier = false;
            CMailTable table;
            ASSERT_TRUE( table.Setup( 2, "life-no-verifier", options ) && table.StartAll() );
            MULTIPLAY::CMatchRecord record = table.MakeRecord( "No check", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, record.info.matchId ) );
            bool refused = false;
            PUMP_WAIT( refused, table.P( 1 ).recorder.CountRefused( "NO_VERIFIER" ) >= 1, 8000 );
            ASSERT_TRUE( refused );
            ASSERT_TRUE( 0 == table.P( 1 ).provider->GetTrustedRevision( record.info.matchId ) );
            MAIL::TMailRefusalVector refusals;
            ASSERT_TRUE( 1 == table.P( 1 ).provider->GetRefusals( record.info.matchId, refusals ) && refusals[ 0 ].code == "NO_VERIFIER" && !refusals[ 0 ].reason.IsNULLOrEmpty() );
        }
        {
            SOptions options;
            options.installVerifier = false;
            options.allowUnverified = true;
            CMailTable table;
            ASSERT_TRUE( table.Setup( 2, "life-allow-unverified", options ) && table.StartAll() );
            MULTIPLAY::CMatchRecord record = table.MakeRecord( "Allowed", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) && table.WaitKnown( 1, record.info.matchId ) && table.WaitTrusted( 1, record.info.matchId, 1 ) );
            ASSERT_TRUE( 0 == table.P( 1 ).recorder.refused.size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Nudge: one reminder per participant per day, only to the player whose turn it is, signed, and a repeat from outside is ignored" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Nudge: one reminder per participant per day, only to the player whose turn it is, signed, and a repeat from outside is ignored" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-nudge" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 0 ).provider->Nudge( id, table.P( 1 ).identity.userId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        bool nudged = false;
        PUMP_WAIT( nudged, table.P( 1 ).recorder.nudges.size() >= 1, 8000 );
        ASSERT_TRUE( nudged && table.P( 1 ).recorder.nudges[ 0 ].matchId == id && table.P( 1 ).recorder.nudges[ 0 ].userId == table.P( 0 ).identity.userId );
        ASSERT_TRUE( 1 == table.CountMatchMails( 1, "NUDGE" ) && 0 == table.CountMatchMails( 2, "NUDGE" ) );

        // again within the day, to someone whose turn it is not, to oneself, to a stranger
        ASSERT_TRUE( table.P( 0 ).provider->Nudge( id, table.P( 1 ).identity.userId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 0 ).provider->Nudge( id, table.P( 2 ).identity.userId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 1 ).provider->Nudge( id, table.P( 1 ).identity.userId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 0 ).provider->Nudge( id, MULTIPLAY::CMatchId::Generate(), future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 0 ).provider->Nudge( MULTIPLAY::CMatchId::Generate(), table.P( 1 ).identity.userId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_NOT_FOUND == result.status );
        table.Settle( 300 );
        ASSERT_TRUE( 1 == table.CountMatchMails( 1, "NUDGE" ) );

        // a day later it is allowed again
        table.Advance( 25 * 60 * 60 * 1000 );
        ASSERT_TRUE( table.P( 0 ).provider->Nudge( id, table.P( 1 ).identity.userId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        PUMP_WAIT( nudged, table.P( 1 ).recorder.nudges.size() >= 2, 8000 );
        ASSERT_TRUE( nudged );

        // a second valid nudge of Alice that comes in the same day is ignored by the receiver: the limit is theirs too
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 0 ).provider->GetMatchInfo( id, info ) );
        MAIL::CMailSignedStatement statement;
        statement.matchId = id;
        statement.statementKind = MAIL::CMailSignedStatement::KindNudge;
        statement.targetUserId = table.P( 1 ).identity.userId;
        statement.revision = 1;
        statement.createdAt = table.clock.nowMs;
        ASSERT_TRUE( statement.Sign( table.P( 0 ).identity ) );
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( statement, data, 64 * 1024 ) );
        MAIL::CMailOutgoing outgoing;
        outgoing.kind = MAIL::MAILKIND_NUDGE;
        outgoing.matchId = id;
        outgoing.revision = 1;
        outgoing.title = "Friday game";
        outgoing.from = COM::CMailAddress( "Alice", "alice@mail.test" );
        outgoing.to.push_back( COM::CMailAddress( "Bob", "bob@mail.test" ) );
        outgoing.transfer = MAIL::MAILTRANSFER_ATTACHMENT;
        outgoing.data = &data;
        outgoing.dataLabel = MAIL::CMailMatchCodec::ArmorLabelData;
        outgoing.dataRevisionName = "nudge";
        COM::CMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CMailMatchCodec::Compose( outgoing, message, reason ) );
        message.messageId = "<manual-nudge@mail.test>";
        COM::CMimeComposer composer;
        CORE::CDynamicBuffer wire;
        ASSERT_TRUE( composer.Compose( message, wire, reason ) );
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", CORE::CString( static_cast< const char* >( wire.GetConstBufferPtr() ), wire.GetDataSize() ) ) );
        table.Settle( 600 );
        ASSERT_TRUE( 2 == table.P( 1 ).recorder.nudges.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resign: a signed leave is mailed to everybody and a forged one is ignored" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Resign: a signed leave is mailed to everybody and a forged one is ignored" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-resign" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // a leave that Mallory signed in the name of Bob
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
        MAIL::CMailSignedStatement forged;
        forged.matchId = id;
        forged.statementKind = MAIL::CMailSignedStatement::KindResign;
        ASSERT_TRUE( forged.Sign( mallory ) );
        forged.userId = table.P( 1 ).identity.userId;
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( forged, data, 64 * 1024 ) );
        MAIL::CMailOutgoing outgoing;
        outgoing.kind = MAIL::MAILKIND_RESIGN;
        outgoing.matchId = id;
        outgoing.title = "Friday game";
        outgoing.from = COM::CMailAddress( "Mallory", "mallory@mail.test" );
        outgoing.to.push_back( COM::CMailAddress( "Alice", "alice@mail.test" ) );
        outgoing.transfer = MAIL::MAILTRANSFER_ATTACHMENT;
        outgoing.data = &data;
        outgoing.dataLabel = MAIL::CMailMatchCodec::ArmorLabelData;
        outgoing.dataRevisionName = "resign";
        COM::CMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CMailMatchCodec::Compose( outgoing, message, reason ) );
        message.messageId = "<forged-resign@mail.test>";
        COM::CMimeComposer composer;
        CORE::CDynamicBuffer wire;
        ASSERT_TRUE( composer.Compose( message, wire, reason ) );
        ASSERT_TRUE( table.world.DeliverTo( "alice@mail.test", CORE::CString( static_cast< const char* >( wire.GetConstBufferPtr() ), wire.GetDataSize() ) ) );
        table.Settle( 600 );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 1 ) );

        // the real one
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 2 ).provider->Resign( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_LEFT == table.StatusOf( 2, id, 2 ) );
        bool left = false;
        PUMP_WAIT( left, MULTIPLAY::MATCHPARTICIPANT_LEFT == table.StatusOf( 0, id, 2 ) && MULTIPLAY::MATCHPARTICIPANT_LEFT == table.StatusOf( 1, id, 2 ), 8000 );
        ASSERT_TRUE( left );
        ASSERT_TRUE( 2 == table.CountMatchMails( 0, "RESIGN" ) );     // the forged one and the real one
        ASSERT_TRUE( 1 == table.CountMatchMails( 1, "RESIGN" ) );
        // the others go on playing; a repeated leave changes nothing
        table.Settle( 300 );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 1 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, id, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Remove: a match is forgotten here with its files, and mail for it that comes later is ignored" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Remove: a match is forgotten here with its files, and mail for it that comes later is ignored" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 2, "life-remove" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 1 ).provider->Remove( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( 1 == table.P( 1 ).recorder.removed.size() && table.P( 1 ).recorder.removed[ 0 ] == id );
        MULTIPLAY::CMatchInfo info;
        ASSERT_FALSE( table.P( 1 ).provider->GetMatchInfo( id, info ) );
        ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( CORE::CombinePath( table.P( 1 ).dir, "matches" ), id ) ) );
        ASSERT_TRUE( table.P( 1 ).provider->Remove( MULTIPLAY::CMatchId::Generate(), future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_NOT_FOUND == result.status );

        // Alice goes on, and a copy of the invitation comes again: neither brings the match back
        ASSERT_TRUE( table.PlayTurn( 1, id, 0, "late", result ) && MULTIPLAY::MATCHOPERATION_NOT_FOUND == result.status );
        GUCEF::vector< TESTFW::CTestImapServer::TMessage > mails = table.P( 1 ).box->imap.GetMessages();
        ASSERT_TRUE( !mails.empty() );
        CORE::CString copy = mails[ 0 ].data;
        const Int32 at = copy.HasSubstr( "Message-ID: <" );
        ASSERT_TRUE( at >= 0 );
        char* raw = const_cast< char* >( copy.C_String() );
        raw[ at + 13 ] = 'x';
        ASSERT_TRUE( table.world.DeliverTo( "bob@mail.test", copy ) );
        table.Settle( 800 );
        ASSERT_FALSE( table.P( 1 ).provider->GetMatchInfo( id, info ) );
        ASSERT_TRUE( 1 == table.P( 1 ).recorder.CountAdded( id ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Rematch: the same participants and settings under a new id, invitations by mail, and the first turn follows by Submit" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Rematch: the same participants and settings under a new id, invitations by mail, and the first turn follows by Submit" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-rematch" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_BODY_TEXT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // not while the match is going on
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 0 ).provider->Rematch( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );

        ASSERT_TRUE( PlayRound( table, id, 1, 3, "b1", false ) );
        ASSERT_TRUE( PlayRound( table, id, 2, 3, "c1", true ) );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_FINISHED == table.MatchStatusOf( 0, id ) );

        ASSERT_TRUE( table.P( 0 ).provider->Rematch( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        const CORE::CString again = result.matchId;
        ASSERT_TRUE( MULTIPLAY::CMatchId::IsValid( again ) && again != id );
        ASSERT_TRUE( table.WaitKnown( 1, again ) && table.WaitKnown( 2, again ) );
        MULTIPLAY::CMatchInfo oldInfo, newInfo;
        ASSERT_TRUE( table.P( 1 ).provider->GetMatchInfo( id, oldInfo ) && table.P( 1 ).provider->GetMatchInfo( again, newInfo ) );
        ASSERT_TRUE( oldInfo.title == newInfo.title && oldInfo.participants.size() == newInfo.participants.size() && oldInfo.rulesetId == newInfo.rulesetId && oldInfo.decisionPolicy == newInfo.decisionPolicy );
        ASSERT_TRUE( newInfo.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ) == "BODY_TEXT" && newInfo.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION ) == "ALL" );
        for ( size_t i=0; i<oldInfo.participants.size(); ++i )
            ASSERT_TRUE( oldInfo.participants[ i ].userId == newInfo.participants[ i ].userId && oldInfo.participants[ i ].publicKey == newInfo.participants[ i ].publicKey && oldInfo.participants[ i ].contact == newInfo.participants[ i ].contact );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_FORMING == newInfo.status && MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, again, 1 ) && 0 == table.P( 1 ).provider->GetTrustedRevision( again ) );
        GUCEF::vector< COM::CMailMessage > invites = MailsOfKind( table, 1, "INVITE" );
        ASSERT_TRUE( 2 == invites.size() );
        bool rematchHeader = false;
        for ( size_t i=0; i<invites.size(); ++i )
            rematchHeader = rematchHeader || invites[ i ].headers.Get( "X-Redux-Rematch-Of" ) == id;
        ASSERT_TRUE( rematchHeader );

        // the first turn of the new match is the creator's, expecting revision 0
        MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game|setup|rematch" );
        MULTIPLAY::CMatchInfo idOnly;
        idOnly.matchId = again;
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( idOnly, table.P( 0 ).identity, package, &table.P( 1 ).identity, MULTIPLAY::MATCHSTATUS_ACTIVE, table.clock.nowMs );
        ASSERT_TRUE( table.P( 0 ).provider->Submit( again, 0, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 1, again, 1 ) && table.WaitTrusted( 2, again, 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Restart: the matches, the position in the chain and the mails already read survive a restart of the provider" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Restart: the matches, the position in the chain and the mails already read survive a restart of the provider" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-restart" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( PlayRound( table, id, 1, 3, "b1", false ) );
        ASSERT_TRUE( PlayRound( table, id, 2, 3, "c1", false ) );

        ASSERT_TRUE( table.Restart( 0 ) );
        ASSERT_TRUE( table.P( 0 ).provider->Start() );
        // the match is announced again from the files, at the revision it had, before any mail is read
        ASSERT_TRUE( 3 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        CProviderRecorder::SUpdate seen;
        ASSERT_TRUE( table.P( 0 ).recorder.GetLast( id, seen ) && 3 == seen.revision && seen.hasInfo && MULTIPLAY::MATCHSTATUS_ACTIVE == seen.status );
        table.Settle( 1000 );
        // the mails that were read before are not read again
        ASSERT_TRUE( 1 == table.P( 0 ).recorder.CountUpdates( id, 3 ) && 0 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );
        // and the chain goes on
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 0, id, 1, "a1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitAllTrusted( id, 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match book: the book of a player shows the invitation, and the turn once, when the mail provider carries the match" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match book: the book of a player shows the invitation, and the turn once, when the mail provider carries the match" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "life-book" ) );
        MULTIPLAY::CMatchBook book;
        TESTFW::CMatchBookRecorder bookRecorder;
        book.SetLocalUserId( table.P( 1 ).identity.userId );
        book.SetClock( &table.clock );
        ASSERT_TRUE( book.AddProvider( table.P( 1 ).ptr ) );
        bookRecorder.SubscribeTo( &book );
        ASSERT_TRUE( table.P( 0 ).provider->Start() && table.P( 2 ).provider->Start() && book.Start() );

        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Book game", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        const CORE::CString id = record.info.matchId;
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 1, result ) );
        bool invited = false;
        PUMP_WAIT( invited, bookRecorder.invites.size() >= 1, 8000 );
        ASSERT_TRUE( invited && bookRecorder.invites[ 0 ] == id );
        ASSERT_TRUE( 0 == book.GetYourTurnCount() );

        ASSERT_TRUE( table.WaitKnown( 2, id ) && table.Respond( 1, id, true, result ) && table.Respond( 2, id, true, result ) );
        bool yourTurn = false;
        PUMP_WAIT( yourTurn, 1 == bookRecorder.CountYourTurn( id, 1 ), 8000 );
        ASSERT_TRUE( yourTurn && 1 == book.GetYourTurnCount() );
        table.Settle( 300 );
        ASSERT_TRUE( 1 == bookRecorder.CountYourTurn( id, 1 ) );

        // Bob plays through the book: the turn goes to Carol and the match leaves the list of Bob's turns
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.MakeTurn( 1, id, 2, "b1", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( book.Submit( id, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( 0 == book.GetYourTurnCount() );
        MULTIPLAY::CMatchEntry entry;
        ASSERT_TRUE( book.GetEntry( id, entry ) && 2 == entry.preview.revision && MULTIPLAY::MATCHGROUP_THEIR_TURN == book.GetGroupOf( entry ) );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
