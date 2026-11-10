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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestMailFixture.h"
#include "TestMailHandshake.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MAILTEST;
using namespace GUCEF::MULTIPLAYPLUGIN;

#define ERRORHERE       MAIL_ERRORHERE
#define ASSERT_TRUE(t)  MAIL_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) MAIL_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 g_day = 24 * 60 * 60 * 1000;

/** The player of the table who has the address, -1 when none */
int
PlayerOfAddress( CEMailTable& table, const CORE::CString& address )
{
    for ( size_t i=0; i<table.players.size(); ++i )
        if ( table.players[ i ]->address == address.Lowercase() )
            return (int) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** The creator starts a draft for the players, by name and address only */
CORE::CString
BeginDraft( CEMailTable& table, UInt32 creator, const char* title, const UInt32* invitees, UInt32 count, CORE::CString* reasonOut = GUCEF_NULL, const GUCEF::map< CORE::CString, CORE::CString >* more = GUCEF_NULL )
{
    const CORE::CString id = MULTIPLAY::CMatchId::Generate();
    GUCEF::vector< MAIL::CEMailIntroContactInput > contacts;
    for ( UInt32 i=0; i<count; ++i )
        contacts.push_back( MAIL::CEMailIntroContactInput( table.P( invitees[ i ] ).name, table.P( invitees[ i ] ).address ) );
    GUCEF::map< CORE::CString, CORE::CString > settings;
    settings[ GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ] = "ATTACHMENT";
    settings[ GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION ] = "ALL";
    if ( GUCEF_NULL != more )
        for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator m = more->begin(); m != more->end(); ++m )
            settings[ (*m).first ] = (*m).second;
    CORE::CString reason;
    const bool ok = table.P( creator ).provider->BeginIntroductions( id, title, settings, contacts, reason );
    if ( GUCEF_NULL != reasonOut )
        *reasonOut = reason;
    return ok ? id : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
HasPending( CEMailTable& table, UInt32 player, const CORE::CString& matchId, MAIL::CEMailInboundIntro* found = GUCEF_NULL )
{
    GUCEF::vector< MAIL::CEMailInboundIntro > pending;
    table.P( player ).provider->GetPendingIntroductions( pending );
    for ( size_t i=0; i<pending.size(); ++i )
        if ( pending[ i ].request.matchId == matchId )
        {
            if ( GUCEF_NULL != found )
                *found = pending[ i ];
            return true;
        }
    return false;
}

bool
WaitPending( CEMailTable& table, UInt32 player, const CORE::CString& matchId, UInt32 timeoutMs = 8000 )
{
    bool ok = false;
    PUMP_WAIT( ok, HasPending( table, player, matchId ), timeoutMs );
    return ok;
}

/*-------------------------------------------------------------------------*/

UInt32
CountIn( CEMailTable& table, UInt32 creator, const CORE::CString& matchId, MAIL::TMailIntroState state )
{
    GUCEF::vector< MAIL::CEMailIntroContact > contacts;
    UInt32 count = 0;
    if ( table.P( creator ).provider->GetIntroductions( matchId, contacts ) )
        for ( size_t i=0; i<contacts.size(); ++i )
            if ( contacts[ i ].state == state )
                ++count;
    return count;
}

bool
WaitCount( CEMailTable& table, UInt32 creator, const CORE::CString& matchId, MAIL::TMailIntroState state, UInt32 count, UInt32 timeoutMs = 8000 )
{
    bool ok = false;
    PUMP_WAIT( ok, CountIn( table, creator, matchId, state ) >= count, timeoutMs );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** What the application does when the draft is complete: the signed record with the keys of the introduced, the first turn, CreateMatch */
bool
StartFromDraft( CEMailTable& table, UInt32 creator, const CORE::CString& matchId, MULTIPLAY::CMatchOperationResult& result, MULTIPLAY::CMatchRecord* recordOut = GUCEF_NULL, const char* changedKey = GUCEF_NULL, const char* changedValue = "" )
{
    MAIL::CEMailIntroDraft draft;
    GUCEF::vector< MAIL::CEMailIntroContact > introduced;
    if ( !table.P( creator ).provider->GetIntroductionDraft( matchId, draft ) )
        return false;
    for ( size_t i=0; i<draft.contacts.size(); ++i )
        if ( MAIL::MAILINTRO_INTRODUCED == draft.contacts[ i ].state )
            introduced.push_back( draft.contacts[ i ] );
    if ( introduced.empty() )
        return false;
    MULTIPLAY::CMatchInfo info;
    info.matchId = matchId;
    info.title = draft.title;
    info.gameId = "testgame";
    info.gameVersion = "1.0";
    info.rulesetId = "standard";
    info.contentHash = "00";
    info.backend = "mail";
    info.mode = MULTIPLAY::MATCHMODE_ASYNC;
    info.status = MULTIPLAY::MATCHSTATUS_FORMING;
    info.createdAt = 1700000000000LL;
    info.creatorUserId = table.P( creator ).identity.userId;
    MULTIPLAY::CMatchParticipant me;
    me.userId = table.P( creator ).identity.userId;
    me.displayName = table.P( creator ).name;
    me.publicKey = table.P( creator ).identity.publicKey;
    me.slot = 0;
    me.status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
    me.contact = table.P( creator ).address;
    info.participants.push_back( me );
    for ( size_t i=0; i<introduced.size(); ++i )
    {
        MULTIPLAY::CMatchParticipant invited;
        invited.publicKey = introduced[ i ].publicKey;
        invited.userId = MULTIPLAY::CPlayerIdentity::MakeUserIdFromHex( invited.publicKey );
        invited.displayName = introduced[ i ].name;
        invited.slot = (UInt32) i + 1;
        invited.status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
        invited.contact = introduced[ i ].address;
        info.participants.push_back( invited );
    }
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = draft.settings.begin(); s != draft.settings.end(); ++s )
        info.SetSetting( (*s).first, (*s).second );
    if ( GUCEF_NULL != changedKey )
        info.SetSetting( changedKey, changedValue );
    MULTIPLAY::CMatchRecord record( info );
    if ( !record.Sign( table.P( creator ).identity ) )
        return false;
    if ( GUCEF_NULL != recordOut )
        *recordOut = record;
    const int holder = PlayerOfAddress( table, introduced[ 0 ].address );
    MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game|setup|start" );
    MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( record.info, table.P( creator ).identity, package, holder >= 0 ? &table.P( (UInt32) holder ).identity : GUCEF_NULL, MULTIPLAY::MATCHSTATUS_ACTIVE, table.clock.nowMs );
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( !table.P( creator ).provider->CreateMatch( record, package, preview, future ) )
        return false;
    return table.Await( future, result );
}

/*-------------------------------------------------------------------------*/

/** Takes a match or a draft away from the provider of a player */
bool
RemoveMatch( CEMailTable& table, UInt32 player, const CORE::CString& matchId )
{
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    MULTIPLAY::CMatchOperationResult result;
    return table.P( player ).provider->Remove( matchId, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status;
}

/*-------------------------------------------------------------------------*/

/** A mail with an introduction statement made by hand, from any address to any player */
CORE::CString
IntroMail( CEMailTable& table, const char* fromAddress, const CORE::CString& toAddress, const CORE::CString& matchId, MAIL::TMailMatchKind kind, const CORE::CIDataNodeSerializable& statement )
{
    CORE::CDynamicBuffer bytes;
    if ( !MAIL::CEMailMatchCodec::SerializeToBytes( statement, bytes, 16 * 1024 ) )
        return CORE::CString::Empty;
    GUCEF::vector< CORE::CString > to;
    to.push_back( toAddress );
    return table.MailTextForData( fromAddress, to, matchId, 0, kind, bytes, MAIL::CEMailMatchCodec::ArmorLabelData, "intro", MAIL::MAILTRANSFER_ATTACHMENT );
}

/*-------------------------------------------------------------------------*/

UInt32
CountCodes( const GUCEF::vector< MAIL::CEMailMatchNotice >& notices, const char* code )
{
    UInt32 count = 0;
    for ( size_t i=0; i<notices.size(); ++i )
        if ( notices[ i ].code == code )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

/** The text of the first mail of a kind in the mailbox of a player */
CORE::CString
FirstMail( CEMailTable& table, UInt32 player, const char* kind )
{
    GUCEF::vector< TESTFW::CTestImapServer::TMessage > messages = table.P( player ).box->imap.GetMessages();
    for ( size_t i=0; i<messages.size(); ++i )
        if ( messages[ i ].data.HasSubstr( CORE::CString( "X-Redux-Kind: " ) + kind + "\r\n" ) >= 0 )
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
PerformMailHandshakeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: introduction by address" );

    GUCEF_TESTFW_TESTCASE( "Handshake: three players with addresses only, the replies arrive in the other order, the game is made from the keys that were introduced, the invitees join at once and the game is played" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: three players with addresses only, the replies arrive in the other order, the game is made from the keys that were introduced, the invitees join at once and the game is played" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        TESTFW::CLogTextCapture logs;
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-three" ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2 };
        const CORE::CString id = BeginDraft( table, 0, "Friday game", invitees, 2 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // the draft is a forming match of the creator, the invitees have to introduce themselves
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 0 ).provider->GetMatchInfo( id, info ) && MULTIPLAY::MATCHSTATUS_INITIALIZING == info.status && info.matchId == id );
        ASSERT_TRUE( table.P( 0 ).recorder.CountAdded( id ) >= 1 );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && 2 == summary.total );
        ASSERT_TRUE( WaitPending( table, 1, id ) && WaitPending( table, 2, id ) );
        ASSERT_TRUE( 1 == table.CountMatchMails( 1, "INTRO_REQUEST" ) && 1 == table.CountMatchMails( 2, "INTRO_REQUEST" ) && 0 == table.CountMatchMails( 1, "INVITE" ) );
        ASSERT_TRUE( CountIn( table, 0, id, MAIL::MAILINTRO_WAITING ) == 2 );

        // the mail is for a person: who invites to which game, with whom, how to get the game, no game contents
        const CORE::CString text = FirstMail( table, 1, "INTRO_REQUEST" );
        ASSERT_TRUE( text.HasSubstr( "X-Redux-Match: " + id ) >= 0 && text.HasSubstr( "X-Redux-Protocol: 1" ) >= 0 && text.HasSubstr( "invites you" ) >= 0 && text.HasSubstr( "Friday game" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "Alice" ) >= 0 && text.HasSubstr( "Carol" ) >= 0 && text.HasSubstr( table.P( 2 ).address ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "see each other's mail address" ) >= 0 && text.HasSubstr( "Redux" ) >= 0 && text.HasSubstr( "games.example.invalid" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( MAIL::CEMailIntroFingerprint::Of( table.P( 0 ).identity.publicKey ) ) >= 0 );
        MAIL::CEMailInboundIntro seenByBob;
        ASSERT_TRUE( HasPending( table, 1, id, &seenByBob ) && seenByBob.request.title == "Friday game" && seenByBob.request.creatorName == "Alice" && seenByBob.request.parties.size() == 3 );
        ASSERT_TRUE( MAIL::CEMailIntroFingerprint::Of( seenByBob.request.creatorPublicKey ) == MAIL::CEMailIntroFingerprint::Of( table.P( 0 ).identity.publicKey ) );

        // Carol answers first, Bob after her; the creator sees both with the short fingerprints
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 2 ).provider->AcceptIntroduction( id, false ) );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) );
        ASSERT_TRUE( CountIn( table, 0, id, MAIL::MAILINTRO_WAITING ) == 1 );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 2 ) );
        GUCEF::vector< MAIL::CEMailIntroContact > contacts;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductions( id, contacts ) && 2 == contacts.size() );
        ASSERT_TRUE( contacts[ 0 ].publicKey == table.P( 1 ).identity.publicKey && contacts[ 1 ].publicKey == table.P( 2 ).identity.publicKey );
        ASSERT_TRUE( MAIL::CEMailIntroFingerprint::Of( contacts[ 0 ].publicKey ) == MAIL::CEMailIntroFingerprint::Of( table.P( 1 ).identity.publicKey ) && MAIL::MAILINTROSOURCE_REPLY == contacts[ 0 ].source );
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && summary.allIntroduced && summary.startable );

        // the game: the record with the pinned keys, the normal invitation with the first turn
        MULTIPLAY::CMatchRecord record;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result, &record ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( record.info.participants[ 1 ].publicKey == table.P( 1 ).identity.publicKey && record.info.participants[ 2 ].publicKey == table.P( 2 ).identity.publicKey );
        // the draft is gone, the match took its place
        GUCEF::vector< MAIL::CEMailIntroContact > gone;
        ASSERT_FALSE( table.P( 0 ).provider->GetIntroductions( id, gone ) );
        ASSERT_TRUE( table.P( 0 ).provider->GetMatchInfo( id, info ) && info.participants.size() == 3 );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );
        // nobody has to answer again: the invitation was agreed to with the introduction
        bool active = false;
        for ( UInt32 p=0; p<3; ++p )
        {
            PUMP_WAIT( active, MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( p, id ), 8000 );
            ASSERT_TRUE( active );
        }
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 1 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 2 ) );
        ASSERT_FALSE( HasPending( table, 1, id ) || HasPending( table, 2, id ) );
        ASSERT_TRUE( 1 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) );
        // and the game is played round the table
        ASSERT_TRUE( table.WaitTrusted( 1, id, 1 ) && table.PlayTurn( 1, id, 2, "b1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) && table.PlayTurn( 2, id, 0, "c1", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 3 ) && table.PlayTurn( 0, id, 1, "a1", result ) );
        ASSERT_TRUE( table.WaitAllTrusted( id, 4 ) );

        // no address, key or nonce was logged by the providers
        ASSERT_FALSE( logs.Contains( table.P( 1 ).identity.publicKey ) || logs.Contains( table.P( 2 ).identity.publicKey ) || logs.Contains( table.P( 0 ).identity.publicKey ) );
        ASSERT_FALSE( logs.Contains( "bob@mail.test" ) || logs.Contains( "carol@mail.test" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: the invitee sees the settings of the game with the request, joins at once when the record has exactly those settings, and waits for an answer of their own when it has others" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: the invitee sees the settings of the game with the request, joins at once when the record has exactly those settings, and waits for an answer of their own when it has others" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-settings" ) && table.StartAll() );
        GUCEF::map< CORE::CString, CORE::CString > more;
        more[ "scenario" ] = "custom";
        more[ "seed" ] = "4242";
        const UInt32 invitees[] = { 1 };
        const CORE::CString same = BeginDraft( table, 0, "Same settings", invitees, 1, GUCEF_NULL, &more );
        const CORE::CString changed = BeginDraft( table, 0, "Changed settings", invitees, 1, GUCEF_NULL, &more );
        ASSERT_FALSE( same.IsNULLOrEmpty() || changed.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, same ) && WaitPending( table, 1, changed ) );

        // what Bob sees before he answers
        MAIL::CEMailInboundIntro seen;
        ASSERT_TRUE( HasPending( table, 1, same, &seen ) );
        ASSERT_TRUE( seen.request.settings.find( "scenario" ) != seen.request.settings.end() && (*seen.request.settings.find( "seed" )).second == "4242" );
        ASSERT_TRUE( seen.request.settings.find( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ) != seen.request.settings.end() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( same, false ) && MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( changed, false ) );
        ASSERT_TRUE( WaitCount( table, 0, same, MAIL::MAILINTRO_INTRODUCED, 1 ) && WaitCount( table, 0, changed, MAIL::MAILINTRO_INTRODUCED, 1 ) );

        // the record has the settings of the request: Bob is in
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, same, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, same ) );
        bool active = false;
        PUMP_WAIT( active, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, same, 1 ), 8000 );
        ASSERT_TRUE( active && 1 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) );

        // the record has another seed than the one Bob was told about: Bob has not agreed to this game and is asked
        ASSERT_TRUE( StartFromDraft( table, 0, changed, result, GUCEF_NULL, "seed", "4243" ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, changed ) );
        table.Settle( 800 );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, changed, 1 ) && 1 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) );
        // and he can accept it by himself
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 1 ).provider->GetMatchInfo( changed, info ) );
        MULTIPLAY::CMatchAcceptance acceptance;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( acceptance.Sign( table.P( 1 ).identity, info, true ) && table.P( 1 ).provider->Respond( changed, acceptance, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        PUMP_WAIT( active, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, changed, 1 ), 8000 );
        ASSERT_TRUE( active );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Handshake: a draft that got fewer seats after the request was sent (a player was taken out) is still the table the others agreed to: they join at once and their state is not asked for again; more seats, or a provider that does not know the seats setting, ask them" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: a draft that got fewer seats after the request was sent (a player was taken out) is still the table the others agreed to: they join at once and their state is not asked for again; more seats, or a provider that does not know the seats setting, ask them" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions options;
        options.seatsSettingKey = "seats";
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-seats", options ) && table.StartAll() );
        // Bob and Carol take part in the game and are told it has four seats; the fourth player left the draft later, so the record has three
        GUCEF::map< CORE::CString, CORE::CString > more;
        more[ "seats" ] = "4";
        const UInt32 invitees[] = { 1, 2 };
        const CORE::CString lowered = BeginDraft( table, 0, "Fewer seats", invitees, 2, GUCEF_NULL, &more );
        const CORE::CString higher = BeginDraft( table, 0, "More seats", invitees, 2, GUCEF_NULL, &more );
        ASSERT_FALSE( lowered.IsNULLOrEmpty() || higher.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, lowered ) && WaitPending( table, 2, lowered ) && WaitPending( table, 1, higher ) && WaitPending( table, 2, higher ) );
        for ( UInt32 p=1; p<3; ++p )
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( p ).provider->AcceptIntroduction( lowered, false ) && MAIL::MAILINTRORESULT_OK == table.P( p ).provider->AcceptIntroduction( higher, false ) );
        ASSERT_TRUE( WaitCount( table, 0, lowered, MAIL::MAILINTRO_INTRODUCED, 2 ) && WaitCount( table, 0, higher, MAIL::MAILINTRO_INTRODUCED, 2 ) );

        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, lowered, result, GUCEF_NULL, "seats", "3" ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, lowered ) && table.WaitKnown( 2, lowered ) );
        bool active = false;
        PUMP_WAIT( active, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, lowered, 1 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, lowered, 2 ), 8000 );
        ASSERT_TRUE( active && 1 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) && 1 == CountCodes( table.P( 2 ).recorder.intros, "INTRO_JOINED" ) );

        // a table with more seats than the players were told is another table
        ASSERT_TRUE( StartFromDraft( table, 0, higher, result, GUCEF_NULL, "seats", "5" ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, higher ) );
        table.Settle( 800 );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, higher, 1 ) && 1 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: without a setting that names the seats a lower number of seats is another table" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: without a setting that names the seats a lower number of seats is another table" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        // without the setting that names the seats a lower number is another table, and a player who turned the automatic acceptance off answers by hand
        SOptions plain;
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-noseats", plain ) && table.StartAll() );
        GUCEF::map< CORE::CString, CORE::CString > more;
        more[ "seats" ] = "4";
        const UInt32 invitees[] = { 1 };
        const CORE::CString id = BeginDraft( table, 0, "Unknown seats", invitees, 1, GUCEF_NULL, &more );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, id ) && MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) && WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result, GUCEF_NULL, "seats", "3" ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, id ) );
        table.Settle( 800 );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, id, 1 ) && 0 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a player whose provider does not accept by itself stays invited to the table that was announced until they accept by hand" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: a player whose provider does not accept by itself stays invited to the table that was announced until they accept by hand" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions manual;
        manual.autoJoinIntroduced = false;
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 2, "handshake-manual", manual ) && table.StartAll() );
        const UInt32 invitees[] = { 1 };
        const CORE::CString id = BeginDraft( table, 0, "Manual", invitees, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, id ) && MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) && WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, id ) );
        table.Settle( 800 );
        // the table is exactly the one that was announced and the player stays invited until they accept by hand
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, id, 1 ) && 0 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) );
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 1 ).provider->GetMatchInfo( id, info ) );
        MULTIPLAY::CMatchAcceptance acceptance;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( acceptance.Sign( table.P( 1 ).identity, info, true ) && table.P( 1 ).provider->Respond( id, acceptance, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        bool active = false;
        PUMP_WAIT( active, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 1, id, 1 ), 8000 );
        ASSERT_TRUE( active );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: duplicated, held back and late replies: a duplicate changes nothing, a held reply arrives later and counts, a reply after the game was started has no draft to join" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: duplicated, held back and late replies: a duplicate changes nothing, a held reply arrives later and counts, a reply after the game was started has no draft to join" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-late" ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2 };
        const CORE::CString id = BeginDraft( table, 0, "Late game", invitees, 2 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, id ) && WaitPending( table, 2, id ) );

        // Bob's reply is delivered twice
        table.world.DuplicateNext( 1 );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) );
        table.Settle( 600 );
        ASSERT_TRUE( CountIn( table, 0, id, MAIL::MAILINTRO_INTRODUCED ) == 1 && CountCodes( table.P( 0 ).recorder.intros, "INTRO_INTRODUCED" ) == 1 );
        ASSERT_TRUE( 0 == CountCodes( table.P( 0 ).recorder.intros, "INTRO_REFUSED" ) );

        // Carol's reply is held back by the mail system
        table.world.SetHold( true, table.P( 0 ).address );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 2 ).provider->AcceptIntroduction( id, false ) );
        table.Settle( 600 );
        ASSERT_TRUE( CountIn( table, 0, id, MAIL::MAILINTRO_WAITING ) == 1 );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && !summary.startable );
        // the creator waits no longer: the game is started with Bob
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( CountIn( table, 0, id, MAIL::MAILINTRO_INTRODUCED ) == 1 );
        table.world.SetHold( false );
        table.world.ReleaseHeld( false );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 2 ) );
        // the held reply counted: both are in, the game has three players
        MULTIPLAY::CMatchRecord record;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result, &record ) && MULTIPLAY::MATCHOPERATION_OK == result.status && 3 == record.info.participants.size() );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );

        // a reply for a draft that was started already: refused with a notice, nothing joins
        MAIL::CEMailIntroReply stray;
        stray.matchId = id;
        stray.nonce = "00112233445566778899aabbccddeeff";
        stray.accepted = false;
        stray.inviteeAddress = table.P( 1 ).address;
        stray.creatorAddress = table.P( 0 ).address;
        stray.createdAt = table.clock.nowMs;
        ASSERT_TRUE( table.Inject( 0, IntroMail( table, table.P( 1 ).address.C_String(), table.P( 0 ).address, id, MAIL::MAILKIND_INTRO_REPLY, stray ) ) );
        bool refused = false;
        PUMP_WAIT( refused, CountCodes( table.P( 0 ).recorder.intros, "INTRO_REFUSED" ) >= 1, 8000 );
        ASSERT_TRUE( refused );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: one player declines, one never answers and the request expires, the creator asks the silent one again with a new nonce, and the game is played by those who introduced" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: one player declines, one never answers and the request expires, the creator asks the silent one again with a new nonce, and the game is played by those who introduced" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SOptions options;
        options.autoClockMs = 0;
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 4, "handshake-expiry", options ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, "Quiet game", invitees, 3 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, id ) && WaitPending( table, 2, id ) && WaitPending( table, 3, id ) );

        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 2 ).provider->DeclineIntroduction( id ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == table.P( 2 ).provider->DeclineIntroduction( id ) );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) && WaitCount( table, 0, id, MAIL::MAILINTRO_DECLINED, 1 ) );
        // the decline is one mail and nothing more is sent by the player who declined
        table.Settle( 400 );
        ASSERT_TRUE( 2 == table.CountMatchMails( 0, "INTRO_REPLY" ) );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && !summary.startable && 1 == summary.waiting );

        // two weeks without an answer
        table.Advance( 15 * g_day, 600 );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_EXPIRED, 1 ) );
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && summary.startable && 1 == summary.introduced && 1 == summary.declined && 1 == summary.expired );
        ASSERT_TRUE( CountCodes( table.P( 0 ).recorder.intros, "INTRO_EXPIRED" ) >= 1 );
        // the request of Dave is no more on Dave's side either
        bool purged = false;
        PUMP_WAIT( purged, !HasPending( table, 3, id ), 8000 );
        ASSERT_TRUE( purged );

        // asked again: a new request that Dave can answer; nobody is bothered twice in an hour
        const UInt32 before = table.CountMatchMails( 3, "INTRO_REQUEST" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 0 ).provider->ResendIntroduction( id, table.P( 3 ).address ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_RATE_LIMITED == table.P( 0 ).provider->ResendIntroduction( id, table.P( 3 ).address ) );
        ASSERT_TRUE( WaitPending( table, 3, id ) );
        table.Settle( 300 );
        ASSERT_TRUE( before + 1 == table.CountMatchMails( 3, "INTRO_REQUEST" ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 3 ).provider->AcceptIntroduction( id, false ) );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 2 ) );
        // Carol declined and is out: the game is for Alice, Bob and Dave
        MULTIPLAY::CMatchOperationResult result;
        MULTIPLAY::CMatchRecord record;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result, &record ) && MULTIPLAY::MATCHOPERATION_OK == result.status && 3 == record.info.participants.size() );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 3, id ) );
        table.Settle( 800 );
        ASSERT_FALSE( table.P( 2 ).provider->GetMatchInfo( id, record.info ) );
        ASSERT_TRUE( 0 == table.CountMatchMails( 2, "INVITE" ) );
        ASSERT_TRUE( table.WaitTrusted( 1, id, 1 ) && table.PlayTurn( 1, id, 3, "b1", result ) );
        ASSERT_TRUE( table.WaitTrusted( 3, id, 2 ) && table.PlayTurn( 3, id, 0, "d1", result ) );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 3 ) && table.PlayTurn( 0, id, 1, "a1", result ) );
        ASSERT_TRUE( table.WaitTrusted( 1, id, 4 ) && table.WaitTrusted( 3, id, 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: forged replies are refused: a wrong key, a wrong nonce and a wrong From introduce nobody, and the notices have codes only" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: forged replies are refused: a wrong key, a wrong nonce and a wrong From introduce nobody, and the notices have codes only" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        TESTFW::CLogTextCapture logs;
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-forged" ) && table.StartAll() );
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
        const UInt32 invitees[] = { 1, 2 };
        const CORE::CString id = BeginDraft( table, 0, "Forged game", invitees, 2 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, id ) );
        MAIL::CEMailInboundIntro seen;
        ASSERT_TRUE( HasPending( table, 1, id, &seen ) );
        const CORE::CString realNonce = seen.request.nonce;

        // Mallory cannot read the nonce: a guess with a good signature of her own key
        MAIL::CEMailIntroReply guessed;
        guessed.matchId = id;
        guessed.nonce = "00112233445566778899aabbccddeeff";
        guessed.accepted = true;
        guessed.displayName = "Bob";
        guessed.inviteeAddress = table.P( 1 ).address;
        guessed.creatorAddress = table.P( 0 ).address;
        guessed.createdAt = table.clock.nowMs;
        ASSERT_TRUE( guessed.Sign( mallory ) );
        ASSERT_TRUE( table.Inject( 0, IntroMail( table, table.P( 1 ).address.C_String(), table.P( 0 ).address, id, MAIL::MAILKIND_INTRO_REPLY, guessed ) ) );
        // the real nonce, but the mail does not come from Bob's address
        MAIL::CEMailIntroReply stolenNonce = guessed;
        stolenNonce.nonce = realNonce;
        ASSERT_TRUE( stolenNonce.Sign( mallory ) );
        ASSERT_TRUE( table.Inject( 0, IntroMail( table, "mallory@mail.test", table.P( 0 ).address, id, MAIL::MAILKIND_INTRO_REPLY, stolenNonce ) ) );
        // the real nonce from Bob's address, claiming Bob's key but signed with Mallory's key
        MAIL::CEMailIntroReply wrongKey = stolenNonce;
        wrongKey.publicKey = table.P( 1 ).identity.publicKey;
        wrongKey.userId = table.P( 1 ).identity.userId;
        ASSERT_TRUE( table.Inject( 0, IntroMail( table, table.P( 1 ).address.C_String(), table.P( 0 ).address, id, MAIL::MAILKIND_INTRO_REPLY, wrongKey ) ) );
        // a mail that says it is an introduction and carries rubbish
        const char rubbish[] = "{ not json at all";
        CORE::CDynamicBuffer junk;
        junk.Append( rubbish, sizeof( rubbish ) - 1 );
        GUCEF::vector< CORE::CString > to;
        to.push_back( table.P( 0 ).address );
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( table.P( 1 ).address.C_String(), to, id, 0, MAIL::MAILKIND_INTRO_REPLY, junk, MAIL::CEMailMatchCodec::ArmorLabelData, "intro", MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        bool refused = false;
        PUMP_WAIT( refused, CountCodes( table.P( 0 ).recorder.intros, "INTRO_REFUSED" ) >= 4, 10000 );
        ASSERT_TRUE( refused );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && 0 == summary.introduced && 2 == summary.waiting );
        // the notices say what kind of refusal, not whose
        for ( size_t i=0; i<table.P( 0 ).recorder.intros.size(); ++i )
        {
            const MAIL::CEMailMatchNotice& notice = table.P( 0 ).recorder.intros[ i ];
            ASSERT_TRUE( notice.description.HasSubstr( "@" ) < 0 && notice.description.HasSubstr( mallory.publicKey ) < 0 && notice.description.HasSubstr( realNonce ) < 0 );
        }
        ASSERT_FALSE( logs.Contains( mallory.publicKey ) || logs.Contains( realNonce ) || logs.Contains( "mallory@mail.test" ) );

        // the real answer is still taken
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) );
        GUCEF::vector< MAIL::CEMailIntroContact > contacts;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductions( id, contacts ) && contacts[ 0 ].publicKey == table.P( 1 ).identity.publicKey );

        // a flood of invitations from one stranger: the player sees three
        for ( int i=0; i<8; ++i )
        {
            MULTIPLAY::CMatchInfo none;
            MAIL::CEMailIntroductions stranger;
            stranger.Configure( mallory, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
            GUCEF::map< CORE::CString, CORE::CString > settings;
            GUCEF::vector< MAIL::CEMailIntroContactInput > one;
            one.push_back( MAIL::CEMailIntroContactInput( "Bob", table.P( 1 ).address ) );
            CORE::CString why;
            const CORE::CString strangerId = MULTIPLAY::CMatchId::Generate();
            ASSERT_TRUE( stranger.BeginDraft( strangerId, "Spam", "Mallory", "mallory@mail.test", settings, one, table.clock.nowMs, why ) );
            MAIL::CEMailIntroRequest request;
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == stranger.PrepareRequest( strangerId, table.P( 1 ).address, false, table.clock.nowMs, request ) );
            ASSERT_TRUE( table.Inject( 1, IntroMail( table, "mallory@mail.test", table.P( 1 ).address, strangerId, MAIL::MAILKIND_INTRO_REQUEST, request ) ) );
        }
        table.Settle( 1500 );
        GUCEF::vector< MAIL::CEMailInboundIntro > pending;
        table.P( 1 ).provider->GetPendingIntroductions( pending );
        UInt32 fromMallory = 0;
        for ( size_t i=0; i<pending.size(); ++i )
            if ( pending[ i ].fromAddress == "mallory@mail.test" )
                ++fromMallory;
        ASSERT_TRUE( 3 == fromMallory );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a key that changed is held for the creator to decide: accepted it is used and the game is made with it, refused the player is left out" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: a key that changed is held for the creator to decide: accepted it is used and the game is made with it, refused the player is left out" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        for ( int variant=0; variant<2; ++variant )
        {
            CEMailTable table;
            ASSERT_TRUE( table.Setup( 2, variant ? "handshake-key-refused" : "handshake-key-accepted" ) && table.StartAll() );
            const UInt32 invitees[] = { 1 };
            // the first game: the key of Bob is met and remembered
            CORE::CString id = BeginDraft( table, 0, "First game", invitees, 1 );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            ASSERT_TRUE( WaitPending( table, 1, id ) && MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) );
            ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 1 ) );
            const CORE::CString oldKey = table.P( 1 ).identity.publicKey;
            ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == table.P( 0 ).provider->GetAddressBook().Check( table.P( 1 ).address, oldKey ) );
            ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == table.P( 1 ).provider->GetAddressBook().Check( table.P( 0 ).address, table.P( 0 ).identity.publicKey ) );
            ASSERT_TRUE( RemoveMatch( table, 0, id ) );

            // Bob came back with another key (a new profile)
            MULTIPLAY::CPlayerIdentity changed;
            ASSERT_TRUE( TESTFW::MakeTestIdentity( "Bob", changed ) );
            table.P( 1 ).provider->SetIdentity( changed );
            table.P( 1 ).identity = changed;
            // the second game: the address is asked again, the key of the book is what the answer is compared with
            id = BeginDraft( table, 0, "Second game", invitees, 1 );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            ASSERT_TRUE( WaitPending( table, 1, id ) );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 1 ).provider->AcceptIntroduction( id, false ) );
            ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_KEY_CHANGED, 1 ) );
            ASSERT_TRUE( CountCodes( table.P( 0 ).recorder.intros, "INTRO_KEY_CHANGED" ) >= 1 );
            MAIL::CEMailIntroSummary summary;
            ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && !summary.startable && 1 == summary.keyChanged );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_FALSE( StartFromDraft( table, 0, id, result ) );                     // nothing to start with: the key is not taken
            ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == table.P( 0 ).provider->GetAddressBook().Check( table.P( 1 ).address, oldKey ) );

            if ( 0 == variant )
            {
                ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 0 ).provider->ConfirmIntroductionKey( id, table.P( 1 ).address, true ) );
                MULTIPLAY::CMatchRecord record;
                ASSERT_TRUE( StartFromDraft( table, 0, id, result, &record ) && MULTIPLAY::MATCHOPERATION_OK == result.status && record.info.participants[ 1 ].publicKey == changed.publicKey );
                ASSERT_TRUE( table.WaitKnown( 1, id ) );
                bool active = false;
                PUMP_WAIT( active, MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( 1, id ), 8000 );
                ASSERT_TRUE( active );
                ASSERT_TRUE( table.WaitTrusted( 1, id, 1 ) && table.PlayTurn( 1, id, 0, "b1", result ) && table.WaitTrusted( 0, id, 2 ) );
            }
            else
            {
                ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == table.P( 0 ).provider->ConfirmIntroductionKey( id, table.P( 1 ).address, false ) );
                ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && 1 == summary.refused && !summary.startable );
                ASSERT_FALSE( StartFromDraft( table, 0, id, result ) );
                ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == table.P( 0 ).provider->GetAddressBook().Check( table.P( 1 ).address, oldKey ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a player is added to a draft that began: checked like the first ones, asked by mail, introduced and in the game like them" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: a player is added to a draft that began: checked like the first ones, asked by mail, introduced and in the game like them" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 3, "handshake-add" ) && table.StartAll() );
        const UInt32 first[] = { 1 };
        const CORE::CString id = BeginDraft( table, 0, "Growing game", first, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        ASSERT_TRUE( WaitPending( table, 1, id ) && 0 == table.CountMatchMails( 2, "INTRO_REQUEST" ) );

        // what is not a player to add: the same address, the maker's own, a bad one, a bad code, a game that is not waiting
        CORE::CString reason;
        MAIL::CEMailMatchProvider& creator = *table.P( 0 ).provider;
        ASSERT_FALSE( creator.AddIntroducedPlayer( id, MAIL::CEMailIntroContactInput( "Again", table.P( 1 ).address ), reason ) );
        ASSERT_FALSE( reason.IsNULLOrEmpty() );
        ASSERT_FALSE( creator.AddIntroducedPlayer( id, MAIL::CEMailIntroContactInput( "Me", table.P( 0 ).address ), reason ) );
        ASSERT_FALSE( creator.AddIntroducedPlayer( id, MAIL::CEMailIntroContactInput( "Nobody", "not an address" ), reason ) );
        ASSERT_FALSE( creator.AddIntroducedPlayer( id, MAIL::CEMailIntroContactInput( "", table.P( 2 ).address ), reason ) );
        ASSERT_FALSE( creator.AddIntroducedPlayer( id, MAIL::CEMailIntroContactInput( "Carol", table.P( 2 ).address, "tooshort" ), reason ) );
        ASSERT_FALSE( creator.AddIntroducedPlayer( MULTIPLAY::CMatchId::Generate(), MAIL::CEMailIntroContactInput( "Carol", table.P( 2 ).address ), reason ) );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( creator.GetIntroductionSummary( id, summary ) && 1 == summary.total );
        ASSERT_TRUE( 0 == table.CountMatchMails( 2, "INTRO_REQUEST" ) );

        // Carol is added: asked at once, with the table as it is now, and the draft is announced again with her
        const size_t updatesBefore = table.P( 0 ).recorder.updated.size();
        ASSERT_TRUE( creator.AddIntroducedPlayer( id, MAIL::CEMailIntroContactInput( table.P( 2 ).name, table.P( 2 ).address ), reason ) );
        ASSERT_TRUE( creator.GetIntroductionSummary( id, summary ) && 2 == summary.total );
        ASSERT_TRUE( table.P( 0 ).recorder.updated.size() > updatesBefore );
        ASSERT_TRUE( WaitPending( table, 2, id ) );
        MAIL::CEMailInboundIntro seenByCarol;
        ASSERT_TRUE( HasPending( table, 2, id, &seenByCarol ) && 3 == seenByCarol.request.parties.size() );

        // the players answer; the game is made with both
        MAIL::TMailIntroResult answer = table.P( 1 ).provider->AcceptIntroduction( id, false );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == answer );
        answer = table.P( 2 ).provider->AcceptIntroduction( id, false );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == answer );
        ASSERT_TRUE( WaitCount( table, 0, id, MAIL::MAILINTRO_INTRODUCED, 2 ) );
        MULTIPLAY::CMatchOperationResult result;
        MULTIPLAY::CMatchRecord record;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result, &record ) && MULTIPLAY::MATCHOPERATION_OK == result.status && 3 == record.info.participants.size() );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );

        // the table has no room for more than five invited players
        const UInt32 more[] = { 1 };
        const CORE::CString second = BeginDraft( table, 0, "Full game", more, 1 );
        ASSERT_FALSE( second.IsNULLOrEmpty() );
        for ( int i=0; i<4; ++i )
            ASSERT_TRUE( creator.AddIntroducedPlayer( second, MAIL::CEMailIntroContactInput( "Extra", ( "extra" + CORE::UInt32ToString( (UInt32) i ) + "@mail.test" ).C_String() ), reason ) );
        ASSERT_FALSE( creator.AddIntroducedPlayer( second, MAIL::CEMailIntroContactInput( "One too many", "toomany@mail.test" ), reason ) );
        ASSERT_TRUE( creator.GetIntroductionSummary( second, summary ) && 5 == summary.total );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a player code is a shortcut: with the codes of all invitees no mail is sent for the introduction and the game starts at once, with the usual invitation to accept" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Handshake: a player code is a shortcut: with the codes of all invitees no mail is sent for the introduction and the game starts at once, with the usual invitation to accept" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        ASSERT_TRUE( table.Setup( 2, "handshake-codes" ) && table.StartAll() );
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        GUCEF::vector< MAIL::CEMailIntroContactInput > contacts;
        contacts.push_back( MAIL::CEMailIntroContactInput( "Bob", table.P( 1 ).address, table.P( 1 ).identity.publicKey ) );
        GUCEF::map< CORE::CString, CORE::CString > settings;
        CORE::CString reason;
        ASSERT_TRUE( table.P( 0 ).provider->BeginIntroductions( id, "Coded", settings, contacts, reason ) );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && summary.allIntroduced && summary.startable );
        table.Settle( 400 );
        ASSERT_TRUE( 0 == table.CountMatchMails( 1, "INTRO_REQUEST" ) );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, id ) );
        // nothing was agreed to by Bob, so he is still asked in the usual way
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_INVITED == table.StatusOf( 1, id, 1 ) );
        ASSERT_TRUE( table.Respond( 1, id, true, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        bool active = false;
        PUMP_WAIT( active, MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( 0, id ), 8000 );
        ASSERT_TRUE( active );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
