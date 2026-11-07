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

#include "matchprovidermail_CMailFiles.h"
#include "TestMailFixture.h"
#include "TestMailChain.h"

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
Addresses( const char* a, const char* b = GUCEF_NULL )
{GUCEF_TRACE;

    GUCEF::vector< CORE::CString > list;
    list.push_back( a );
    if ( GUCEF_NULL != b )
        list.push_back( b );
    return list;
}

/*-------------------------------------------------------------------------*/

bool
FileExists( const CMailTable& table, UInt32 player, const CORE::CString& matchId, const CORE::CString& file )
{GUCEF_TRACE;

    return !file.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( table.players[ player ]->dir, "matches" ), matchId ), file ) );
}

/*-------------------------------------------------------------------------*/

/** The wire text of the n-th message of a mailbox that is a turn of the revision */
CORE::CString
FindTurnMail( CMailTable& table, UInt32 player, const char* revision )
{GUCEF_TRACE;

    GUCEF::vector< TESTFW::CTestImapServer::TMessage > messages = table.players[ player ]->box->imap.GetMessages();
    for ( size_t i=0; i<messages.size(); ++i )
        if ( messages[ i ].data.HasSubstr( "X-Redux-Kind: TURN\r\n" ) >= 0 && messages[ i ].data.HasSubstr( CORE::CString( "X-Redux-Revision: " ) + revision + "\r\n" ) >= 0 )
            return messages[ i ].data;
    return CORE::CString::Empty;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailChainTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: lost, late, repeated, forged and wrong turns" );

    GUCEF_TESTFW_TESTCASE( "Lost mail: the outbox sends it again after the quiet period and the receivers catch up; the resend operation and the acknowledgement end it" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lost mail: the outbox sends it again after the quiet period and the receivers catch up; the resend operation and the acknowledgement end it" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-lost" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        table.world.DropNext( 1, "X-Redux-Kind: TURN" );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        bool sent = false;
        PUMP_WAIT( sent, MAIL::MAILDELIVERY_SENT == table.P( 1 ).provider->GetDeliveryState( id, 2 ), 8000 );
        ASSERT_TRUE( sent );
        table.Settle( 600 );
        // the mail server took it and nobody got it
        ASSERT_TRUE( 1 == table.P( 2 ).provider->GetTrustedRevision( id ) && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        ASSERT_TRUE( MAIL::MAILDELIVERY_SENT == table.P( 1 ).provider->GetDeliveryState( id, 2 ) );

        // before the quiet period nothing happens
        table.Advance( 30000, 400 );
        ASSERT_TRUE( 1 == table.P( 2 ).provider->GetTrustedRevision( id ) );
        // after it the same mail is sent again
        table.Advance( 31000, 200 );
        ASSERT_TRUE( table.WaitAllTrusted( id, 2 ) );
        UInt32 attempts = 0;
        bool delivered = false;
        PUMP_WAIT( delivered, MAIL::MAILDELIVERY_DELIVERED == table.P( 1 ).provider->GetDeliveryState( id, 2, &attempts ), 8000 );
        ASSERT_TRUE( delivered );
        ASSERT_TRUE( 1 == table.P( 2 ).recorder.CountUpdates( id, 2 ) && 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );

        // the player can ask for a resend of a turn at any time: receivers hold it already and say so once more
        const UInt32 mailsAtCarol = table.CountMatchMails( 2, "TURN" );
        ASSERT_TRUE( table.P( 1 ).provider->Resend( id, 2 ) );
        bool again = false;
        PUMP_WAIT( again, table.CountMatchMails( 2, "TURN" ) > mailsAtCarol, 8000 );
        ASSERT_TRUE( again );
        table.Settle( 400 );
        ASSERT_TRUE( 1 == table.P( 2 ).recorder.CountUpdates( id, 2 ) && 2 == table.P( 2 ).provider->GetTrustedRevision( id ) );
        ASSERT_FALSE( table.P( 1 ).provider->Resend( id, 9 ) );
        ASSERT_FALSE( table.P( 1 ).provider->Resend( MULTIPLAY::CMatchId::Generate(), 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wrong order: the next turn arriving before the one it follows waits for it and both are applied in order" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Wrong order: the next turn arriving before the one it follows waits for it and both are applied in order" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-order" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        table.world.SetHold( true, "alice@mail.test" );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 2, id, 2 ) );
        ASSERT_TRUE( table.PlayTurn( 2, id, 1, "c1", result ) && table.WaitTrusted( 1, id, 3 ) );
        table.Settle( 300 );
        ASSERT_TRUE( 2 == table.world.GetHeldCount() && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );

        // the later one first
        table.world.SetHold( false );
        table.world.ReleaseHeld( true );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 3 ) );
        // applied in order, each once, and nothing was refused
        ASSERT_TRUE( 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) && 1 == table.P( 0 ).recorder.CountUpdates( id, 3 ) && 0 == table.P( 0 ).recorder.refused.size() );
        size_t at2 = 0, at3 = 0;
        for ( size_t i=0; i<table.P( 0 ).recorder.updated.size(); ++i )
        {
            if ( table.P( 0 ).recorder.updated[ i ].revision == 2 ) at2 = i + 1;
            if ( table.P( 0 ).recorder.updated[ i ].revision == 3 ) at3 = i + 1;
        }
        ASSERT_TRUE( at2 > 0 && at3 > at2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Duplicate mail: delivered twice, applied once" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Duplicate mail: delivered twice, applied once" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-duplicate" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        table.world.DuplicateNext( 1 );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 2, id, 2 ) && table.WaitTrusted( 0, id, 2 ) );
        table.Settle( 800 );
        ASSERT_TRUE( 2 == table.CountMatchMails( 2, "TURN" ) );
        ASSERT_TRUE( 1 == table.P( 2 ).recorder.CountUpdates( id, 2 ) && 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );
        ASSERT_TRUE( 0 == table.P( 2 ).recorder.refused.size() && 0 == table.P( 0 ).recorder.refused.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Old package: a turn that is replayed later changes nothing" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Old package: a turn that is replayed later changes nothing" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-replay" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitAllTrusted( id, 2 ) );
        ASSERT_TRUE( table.PlayTurn( 2, id, 0, "c1", result ) && table.WaitAllTrusted( id, 3 ) );
        ASSERT_TRUE( table.PlayTurn( 0, id, 1, "a1", result ) && table.WaitAllTrusted( id, 4 ) );
        table.Settle( 400 );

        // the second revision again, as a new mail
        CORE::CString old = FindTurnMail( table, 2, "2" );
        ASSERT_FALSE( old.IsNULLOrEmpty() );
        const Int32 at = old.HasSubstr( "Message-ID: <" );
        ASSERT_TRUE( at >= 0 );
        const_cast< char* >( old.C_String() )[ at + 13 ] = 'r';
        const UInt32 updatesCarol = (UInt32) table.P( 2 ).recorder.updated.size();
        const UInt32 updatesAlice = (UInt32) table.P( 0 ).recorder.updated.size();
        ASSERT_TRUE( table.Inject( 2, old ) && table.Inject( 0, old ) );
        table.Settle( 800 );
        ASSERT_TRUE( 4 == table.P( 2 ).provider->GetTrustedRevision( id ) && 4 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        ASSERT_TRUE( updatesCarol == table.P( 2 ).recorder.updated.size() && updatesAlice == table.P( 0 ).recorder.updated.size() );
        ASSERT_TRUE( 0 == table.P( 2 ).recorder.refused.size() && 0 == table.P( 0 ).recorder.refused.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Equivocation: two signed successors of one revision are shown as proof, the first valid one stays, and the second is kept and never applied" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Equivocation: two signed successors of one revision are shown as proof, the first valid one stays, and the second is kept and never applied" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-equivocation" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // Alice gets the second form of the second revision first; the real mail for her waits
        MULTIPLAY::CMatchPackage evilPackage;
        MULTIPLAY::CMatchPreview evilPreview;
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 0, id, 0, "b1-other", evilPackage, evilPreview ) );
        table.world.SetHold( true, "alice@mail.test" );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 2, id, 2 ) );
        ASSERT_TRUE( table.Inject( 0, table.TurnMailText( "bob@mail.test", Addresses( "alice@mail.test" ), id, evilPreview, evilPackage, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 2 ) );
        MULTIPLAY::CMatchPreview heldPreview;
        MULTIPLAY::CMatchPackage heldPackage;
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 2, heldPreview, heldPackage ) );
        ASSERT_TRUE( CGameVerifier::Text( heldPackage ) == "game|setup|start|b1-other" );
        table.world.SetHold( false );
        table.world.ReleaseHeld( false );
        bool proof = false;
        PUMP_WAIT( proof, table.P( 0 ).recorder.equivocations.size() >= 1, 8000 );
        ASSERT_TRUE( proof );
        // first valid wins: Alice keeps the form she saw first
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 2, heldPreview, heldPackage ) && CGameVerifier::Text( heldPackage ) == "game|setup|start|b1-other" );
        MAIL::TMailEquivocationVector proofs;
        ASSERT_TRUE( 1 == table.P( 0 ).provider->GetEquivocations( id, proofs ) && 2 == proofs[ 0 ].revision && proofs[ 0 ].issuerUserId == table.P( 1 ).identity.userId );
        ASSERT_TRUE( proofs[ 0 ].firstSha256 != proofs[ 0 ].secondSha256 && FileExists( table, 0, id, proofs[ 0 ].file ) );
        ASSERT_TRUE( table.P( 0 ).recorder.equivocations[ 0 ].matchId == id && table.P( 0 ).recorder.equivocations[ 0 ].userId == table.P( 1 ).identity.userId );
        ASSERT_TRUE( 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );

        // Carol has the real one; the other form reaches her too: the same proof
        ASSERT_TRUE( table.Inject( 2, table.TurnMailText( "bob@mail.test", Addresses( "carol@mail.test" ), id, evilPreview, evilPackage, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        PUMP_WAIT( proof, table.P( 2 ).recorder.equivocations.size() >= 1, 8000 );
        ASSERT_TRUE( proof );
        ASSERT_TRUE( table.P( 2 ).provider->GetStoredTurn( id, 2, heldPreview, heldPackage ) && CGameVerifier::Text( heldPackage ) == "game|setup|start|b1" );
        ASSERT_TRUE( 1 == table.P( 2 ).recorder.CountUpdates( id, 2 ) );
        // and the turn that follows is built on one of them: it only fits one
        MULTIPLAY::CMatchRecord unused;
        MULTIPLAY::CMatchPackage c1;
        MULTIPLAY::CMatchPreview c1Preview;
        ASSERT_TRUE( table.MakeTurn( 2, id, 0, "c1", MULTIPLAY::MATCHSTATUS_ACTIVE, c1, c1Preview ) );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( table.P( 2 ).provider->Submit( id, 2, c1Preview, c1, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Forged turns: a stranger, the wrong player, a changed container and a good turn from a strange sender" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Forged turns: a stranger, the wrong player, a changed container and a good turn from a strange sender" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-forged" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );

        // a stranger with a good signature of their own, mailed in the name of Bob
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.CraftTurn( mallory, 2, id, 0, "m1", package, preview ) );
        ASSERT_TRUE( table.Inject( 2, table.TurnMailText( "bob@mail.test", Addresses( "carol@mail.test" ), id, preview, package, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        bool refused = false;
        PUMP_WAIT( refused, table.P( 2 ).recorder.CountRefused( "UNKNOWN_SIGNER" ) >= 1, 8000 );
        ASSERT_TRUE( refused );
        ASSERT_TRUE( 1 == table.P( 2 ).provider->GetTrustedRevision( id ) );
        MAIL::TMailRefusalVector refusals;
        ASSERT_TRUE( 1 == table.P( 2 ).provider->GetRefusals( id, refusals ) && refusals[ 0 ].code == "UNKNOWN_SIGNER" && 2 == refusals[ 0 ].revision && !refusals[ 0 ].reason.IsNULLOrEmpty() && FileExists( table, 2, id, refusals[ 0 ].file ) );
        ASSERT_TRUE( table.P( 2 ).recorder.refused[ 0 ].matchId == id && 2 == table.P( 2 ).recorder.refused[ 0 ].revision );

        // Carol plays a turn that was Bob's
        ASSERT_TRUE( table.CraftTurn( table.P( 2 ).identity, 0, id, 0, "c-too-early", package, preview ) );
        ASSERT_TRUE( table.Inject( 0, table.TurnMailText( "carol@mail.test", Addresses( "alice@mail.test" ), id, preview, package, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        PUMP_WAIT( refused, table.P( 0 ).recorder.CountRefused( "NOT_ON_TURN" ) >= 1, 8000 );
        ASSERT_TRUE( refused && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );

        // a container with a changed byte
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 0, id, 2, "b-damaged", package, preview ) );
        CORE::CDynamicBuffer container;
        ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( preview, package, false, container ) );
        static_cast< UInt8* >( container.GetBufferPtr() )[ container.GetDataSize() - 3 ] ^= 0x40;
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "bob@mail.test", Addresses( "alice@mail.test" ), id, 2, MAIL::MAILKIND_TURN, container, MAIL::CMailMatchCodec::ArmorLabelTurn, "r2", MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        PUMP_WAIT( refused, table.P( 0 ).recorder.CountRefused( "BAD_CONTAINER" ) >= 1, 8000 );
        ASSERT_TRUE( refused && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );

        // a good turn of Bob that reaches Alice by way of someone else is still a good turn: the signature decides, not the sender
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 0, id, 2, "b1", package, preview ) );
        ASSERT_TRUE( table.Inject( 0, table.TurnMailText( "somebody.else@elsewhere.test", Addresses( "alice@mail.test" ), id, preview, package, MAIL::MAILTRANSFER_BODY_TEXT ) ) );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 2 ) );
        MULTIPLAY::CMatchPreview stored;
        MULTIPLAY::CMatchPackage storedPackage;
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 2, stored, storedPackage ) && stored.VerifyWithKey( table.P( 1 ).identity.publicKey ) );
        // a mail for a match that is unknown here is kept for when the match is known, and does nothing now
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 0, id, 2, "b-again", package, preview ) );
        MULTIPLAY::CMatchPackage strangePackage = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game|setup|elsewhere" );
        MULTIPLAY::CMatchInfo strangeInfo;
        strangeInfo.matchId = MULTIPLAY::CMatchId::Generate();
        MULTIPLAY::CMatchPreview strangePreview = TESTFW::MakeTestPreview( strangeInfo, table.P( 1 ).identity, strangePackage, &table.P( 2 ).identity, MULTIPLAY::MATCHSTATUS_ACTIVE, 5 );
        ASSERT_TRUE( table.Inject( 2, table.TurnMailText( "bob@mail.test", Addresses( "carol@mail.test" ), strangeInfo.matchId, strangePreview, strangePackage, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        table.Settle( 800 );
        MULTIPLAY::CMatchInfo info;
        ASSERT_FALSE( table.P( 2 ).provider->GetMatchInfo( strangeInfo.matchId, info ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Game check: a turn the game does not accept is kept with the reason, announced and never applied; a base that is missing is asked for" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Game check: a turn the game does not accept is kept with the reason, announced and never applied; a base that is missing is asked for" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        {
            CMailTable table;
            ASSERT_TRUE( table.Setup( 3, "chain-game-rejects" ) && table.StartAll() );
            const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            // Bob's client allowed a move that is not legal: his own application applied it, the others replay it
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.PlayTurn( 1, id, 2, "CHEAT", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
            bool refused = false;
            PUMP_WAIT( refused, table.P( 2 ).recorder.CountRefused( "GAME_REJECTED" ) >= 1 && table.P( 0 ).recorder.CountRefused( "GAME_REJECTED" ) >= 1, 8000 );
            ASSERT_TRUE( refused );
            ASSERT_TRUE( 1 == table.P( 2 ).provider->GetTrustedRevision( id ) && 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );
            ASSERT_TRUE( 0 == table.P( 2 ).recorder.CountUpdates( id, 2 ) );
            MAIL::TMailRefusalVector refusals;
            ASSERT_TRUE( 1 == table.P( 2 ).provider->GetRefusals( id, refusals ) );
            ASSERT_TRUE( refusals[ 0 ].reason.HasSubstr( "not legal" ) >= 0 && FileExists( table, 2, id, refusals[ 0 ].file ) && refusals[ 0 ].at > 0 );
            // the refusal is not announced again when the mail comes again
            table.Advance( 61000, 800 );
            ASSERT_TRUE( 1 == table.P( 2 ).recorder.CountRefused( "GAME_REJECTED" ) );
        }
        {
            CMailTable table;
            ASSERT_TRUE( table.Setup( 3, "chain-no-base" ) && table.StartAll() );
            const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            table.P( 2 ).verifier.noBaseCount = 1;
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 0, id, 2 ) );
            // Carol could not judge it at first, asked for the base, got it and applied the turn
            ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) );
            ASSERT_TRUE( table.CountMatchMails( 0, "RESEND" ) + table.CountMatchMails( 1, "RESEND" ) >= 1 );
            ASSERT_TRUE( 0 == table.P( 2 ).recorder.refused.size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Size: a turn above the cap is refused when sent, and a message above the cap is not even fetched" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Size: a turn above the cap is refused when sent, and a message above the cap is not even fetched" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-size" ) );
        // Alice reads at most 8 KB messages and containers of 4 KB; Bob sends at most 4 KB
        table.P( 0 ).settings.maxMessageBytes = 8192;
        ASSERT_TRUE( table.P( 0 ).provider->SetSettings( table.P( 0 ).settings ) );
        table.P( 1 ).settings.maxContainerBytes = 4096;
        ASSERT_TRUE( table.P( 1 ).provider->SetSettings( table.P( 1 ).settings ) );
        ASSERT_TRUE( table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // 9000 characters that hardly compress
        MULTIPLAY::CMatchPackage base;
        MULTIPLAY::CMatchPreview basePreview;
        ASSERT_TRUE( table.P( 1 ).provider->GetStoredTurn( id, 1, basePreview, base ) );
        CORE::CString big = CGameVerifier::Text( base ) + "|";
        UInt32 state = 5;
        static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        for ( int i=0; i<9000; ++i )
        {
            state = state * 1664525u + 1013904223u;
            big += alphabet[ ( state >> 24 ) % 62 ];
        }
        MULTIPLAY::CMatchPackage bigPackage = TESTFW::MakeTestPackage( 2, &base, big.C_String() );
        MULTIPLAY::CMatchInfo idOnly;
        idOnly.matchId = id;
        MULTIPLAY::CMatchPreview bigPreview = TESTFW::MakeTestPreview( idOnly, table.P( 1 ).identity, bigPackage, &table.P( 2 ).identity, MULTIPLAY::MATCHSTATUS_ACTIVE, 5 );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 1 ).provider->Submit( id, 1, bigPreview, bigPackage, future ) && table.Await( future, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status && result.reason.HasSubstr( "large" ) >= 0 );
        ASSERT_TRUE( 1 == table.P( 1 ).provider->GetTrustedRevision( id ) );

        // Carol, whose limits are the largest, has the real thing from Bob by hand; the mail to Alice is above her cap
        MULTIPLAY::CMatchPackage mailPackage;
        MULTIPLAY::CMatchPreview mailPreview;
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 0, id, 2, "tiny", mailPackage, mailPreview ) );
        CORE::CDynamicBuffer container;
        ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( bigPreview, bigPackage, true, container ) && container.GetDataSize() > 5000 );
        const UInt32 fetchesBefore = (UInt32) table.P( 0 ).box->imap.GetCommands().size();
        (void) fetchesBefore;
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "bob@mail.test", Addresses( "alice@mail.test" ), id, 2, MAIL::MAILKIND_TURN, container, MAIL::CMailMatchCodec::ArmorLabelTurn, "r2", MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        bool problem = false;
        PUMP_WAIT( problem, table.P( 0 ).recorder.CountRefused( "TOO_LARGE" ) >= 1 || table.P( 0 ).recorder.problems.size() >= 1, 8000 );
        ASSERT_TRUE( problem );
        ASSERT_TRUE( 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );
        // the message was never asked for: only its headers were
        GUCEF::vector< CORE::CString > commands = table.P( 0 ).box->imap.GetCommands();
        UInt32 headerFetches = 0, bodyFetches = 0;
        for ( size_t i=0; i<commands.size(); ++i )
        {
            if ( commands[ i ].HasSubstr( "FETCH" ) >= 0 && commands[ i ].HasSubstr( "HEADER" ) >= 0 ) ++headerFetches;
            else if ( commands[ i ].HasSubstr( "FETCH" ) >= 0 && commands[ i ].HasSubstr( "BODY.PEEK[]" ) >= 0 ) ++bodyFetches;
        }
        ASSERT_TRUE( headerFetches >= 1 );
        // the two mails that were read completely are the invitation and the first turn of the match
        GUCEF::vector< TESTFW::CTestImapServer::TMessage > mails = table.P( 0 ).box->imap.GetMessages();
        UInt32 bigUid = 0;
        for ( size_t i=0; i<mails.size(); ++i )
            if ( mails[ i ].data.ByteSize() > 8192 )
                bigUid = mails[ i ].uid;
        ASSERT_TRUE( 0 != bigUid );
        for ( size_t i=0; i<commands.size(); ++i )
            ASSERT_FALSE( commands[ i ].HasSubstr( "FETCH " + CORE::UInt32ToString( bigUid ) + " " ) >= 0 && commands[ i ].HasSubstr( "BODY.PEEK[]" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Removed attachment: a mail server that strips attachments is answered with a request for the text form, and the turn arrives" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Removed attachment: a mail server that strips attachments is answered with a request for the text form, and the turn arrives" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-stripped" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        table.world.SetStripAttachments( true );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        // the mail came without its data; Carol asks for the text form
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2, 15000 ) );
        GUCEF::vector< TESTFW::CTestImapServer::TMessage > mails = table.P( 1 ).box->imap.GetMessages();
        bool asked = false;
        for ( size_t i=0; i<mails.size(); ++i )
            asked = asked || ( mails[ i ].data.HasSubstr( "X-Redux-Kind: RESEND" ) >= 0 && mails[ i ].data.HasSubstr( "X-Redux-Transfer: BODY_TEXT" ) >= 0 );
        ASSERT_TRUE( asked );
        // the answer is text in the body
        mails = table.P( 2 ).box->imap.GetMessages();
        bool text = false;
        for ( size_t i=0; i<mails.size(); ++i )
            text = text || mails[ i ].data.HasSubstr( "-----BEGIN MATCH TURN-----" ) >= 0;
        ASSERT_TRUE( text );
        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        ASSERT_TRUE( table.P( 2 ).provider->GetStoredTurn( id, 2, preview, package ) && preview.VerifyWithKey( table.P( 1 ).identity.publicKey ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Export and import: a turn goes into a file or a text and comes in as if it had arrived by mail, with the same checks" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Export and import: a turn goes into a file or a text and comes in as if it had arrived by mail, with the same checks" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-import" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_NEXT_ONLY, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 2, id, 2 ) );

        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        ASSERT_TRUE( table.P( 1 ).provider->GetStoredTurn( id, 2, preview, package ) );
        const CORE::CString dir = CORE::CombinePath( table.root, "export" );
        CORE::CreateDirs( dir );
        const CORE::CString path = CORE::CombinePath( dir, MAIL::CMailMatchCodec::MakeAttachmentName( id, "r2" ) );
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CMailMatchProvider::ExportToFile( preview, package, path, reason ) && CORE::FileExists( path ) );
        // only the file is there, no temporary file
        CORE::CString::StringVector names;
        MAIL::CMailFiles::ListFiles( dir, names );
        ASSERT_TRUE( 1 == names.size() );
        CORE::CDynamicBuffer fileBytes;
        ASSERT_TRUE( MAIL::CMailFiles::ReadBounded( path, 3 * 1024 * 1024, fileBytes ) );
        MULTIPLAY::CMatchContainer decoded;
        ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == decoded.Decode( fileBytes.GetConstBufferPtr(), fileBytes.GetDataSize() ) && decoded.package.sha256 == package.sha256 );

        // damaged, then too large, then a file that is no turn, then of nobody's match
        CORE::CDynamicBuffer damaged = fileBytes;
        static_cast< UInt8* >( damaged.GetBufferPtr() )[ damaged.GetDataSize() - 2 ] ^= 1;
        const CORE::CString damagedPath = CORE::CombinePath( dir, "damaged.rdxturn" );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( damagedPath, damaged.GetConstBufferPtr(), damaged.GetDataSize() ) );
        ASSERT_FALSE( table.P( 0 ).provider->ImportFile( damagedPath, reason ) || reason.IsNULLOrEmpty() );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( CORE::CombinePath( dir, "text.rdxturn" ), "hello", 5 ) );
        ASSERT_FALSE( table.P( 0 ).provider->ImportFile( CORE::CombinePath( dir, "text.rdxturn" ), reason ) );
        ASSERT_FALSE( table.P( 0 ).provider->ImportFile( CORE::CombinePath( dir, "missing.rdxturn" ), reason ) );
        ASSERT_FALSE( table.P( 0 ).provider->ImportText( "no block in here", reason ) );
        CORE::CDynamicBuffer huge;
        huge.SetDataSize( 3 * 1024 * 1024 );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( CORE::CombinePath( dir, "huge.rdxturn" ), huge.GetConstBufferPtr(), huge.GetDataSize() ) );
        ASSERT_FALSE( table.P( 0 ).provider->ImportFile( CORE::CombinePath( dir, "huge.rdxturn" ), reason ) );
        ASSERT_TRUE( 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );

        // the good file: Alice never got the mail and takes the file
        ASSERT_TRUE( table.P( 0 ).provider->ImportFile( path, reason ) );
        ASSERT_TRUE( 2 == table.P( 0 ).provider->GetTrustedRevision( id ) && 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );
        // a turn signed by a stranger is refused on this way in as well
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
        MULTIPLAY::CMatchPackage strangerPackage;
        MULTIPLAY::CMatchPreview strangerPreview;
        ASSERT_TRUE( table.CraftTurn( mallory, 2, id, 0, "m", strangerPackage, strangerPreview ) );
        const CORE::CString strangerPath = CORE::CombinePath( dir, "stranger.rdxturn" );
        ASSERT_TRUE( MAIL::CMailMatchProvider::ExportToFile( strangerPreview, strangerPackage, strangerPath, reason ) );
        ASSERT_FALSE( table.P( 2 ).provider->ImportFile( strangerPath, reason ) || reason.IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == table.P( 2 ).recorder.CountRefused( "UNKNOWN_SIGNER" ) );
        // a bad target of an export
        ASSERT_FALSE( MAIL::CMailMatchProvider::ExportToFile( preview, package, CORE::CombinePath( CORE::CombinePath( dir, "no-such-dir" ), "x/y.rdxturn" ), reason ) && !CORE::FileExists( CORE::CombinePath( CORE::CombinePath( dir, "no-such-dir" ), "x/y.rdxturn" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Recovery from a third player: a missing turn is asked from the table and any participant that holds it answers with the original signed turn" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Recovery from a third player: a missing turn is asked from the table and any participant that holds it answers with the original signed turn" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "chain-third" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_NEXT_ONLY, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.PlayTurn( 1, id, 2, "b1", result ) && table.WaitTrusted( 2, id, 2 ) );
        table.Settle( 400 );
        // Bob is gone from the table
        table.P( 1 ).provider->Stop();
        ASSERT_TRUE( table.PlayTurn( 2, id, 0, "c1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 3, 15000 ) );
        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        ASSERT_TRUE( table.P( 0 ).provider->GetStoredTurn( id, 2, preview, package ) );
        // Carol sent it, Bob signed it, and nobody changed it
        ASSERT_TRUE( preview.VerifyWithKey( table.P( 1 ).identity.publicKey ) && preview.issuerKey == table.P( 1 ).identity.publicKey );
        ASSERT_TRUE( CGameVerifier::Text( package ) == "game|setup|start|b1" );
        ASSERT_TRUE( 0 == table.P( 1 ).recorder.CountUpdates( id, 3 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
