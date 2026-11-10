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

#include "TestMailFixture.h"
#include "TestMailDraftVotes.h"

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

/** The creator starts a draft for the players, by name and address only; the table has four seats */
CORE::CString
BeginDraft( CEMailTable& table, UInt32 creator, const UInt32* invitees, UInt32 count, const char* policy = GUCEF_NULL )
{
    const CORE::CString id = MULTIPLAY::CMatchId::Generate();
    GUCEF::vector< MAIL::CEMailIntroContactInput > contacts;
    for ( UInt32 i=0; i<count; ++i )
        contacts.push_back( MAIL::CEMailIntroContactInput( table.P( invitees[ i ] ).name, table.P( invitees[ i ] ).address ) );
    GUCEF::map< CORE::CString, CORE::CString > settings;
    settings[ GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ] = "ATTACHMENT";
    settings[ GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION ] = "ALL";
    settings[ "seats" ] = CORE::UInt32ToString( count + 1 );
    if ( GUCEF_NULL != policy )
        settings[ "decisionPolicy" ] = policy;
    CORE::CString reason;
    return table.P( creator ).provider->BeginIntroductions( id, "Friday game", settings, contacts, reason ) ? id : CORE::CString::Empty;
}

bool
HasPending( CEMailTable& table, UInt32 player, const CORE::CString& matchId, MAIL::CEMailInboundIntro::EStatus* status = GUCEF_NULL )
{
    GUCEF::vector< MAIL::CEMailInboundIntro > pending;
    table.P( player ).provider->GetPendingIntroductions( pending );
    for ( size_t i=0; i<pending.size(); ++i )
        if ( pending[ i ].request.matchId == matchId )
        {
            if ( GUCEF_NULL != status )
                *status = pending[ i ].status;
            return true;
        }
    return false;
}

bool
WaitPending( CEMailTable& table, UInt32 player, const CORE::CString& matchId )
{
    bool ok = false;
    PUMP_WAIT( ok, HasPending( table, player, matchId ), 8000 );
    return ok;
}

/** The invitees ask to be introduced, the listed ones accept */
bool
IntroduceAll( CEMailTable& table, const CORE::CString& id, const UInt32* accepting, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( !WaitPending( table, accepting[ i ], id ) || MAIL::MAILINTRORESULT_OK != table.P( accepting[ i ] ).provider->AcceptIntroduction( id, false ) )
            return false;
    return true;
}

/** The number of participants who are introduced and do not watch, in the roster that the player holds */
UInt32
VotersOf( CEMailTable& table, UInt32 player, const CORE::CString& id )
{
    MULTIPLAY::CMatchRoster roster;
    return table.P( player ).provider->GetDraftRoster( id, roster ) ? roster.CountVoters() : 0;
}

bool
WaitVoters( CEMailTable& table, UInt32 player, const CORE::CString& id, UInt32 voters )
{
    bool ok = false;
    PUMP_WAIT( ok, VotersOf( table, player, id ) == voters, 10000 );
    return ok;
}

bool
GetTally( CEMailTable& table, UInt32 player, const CORE::CString& id, const CORE::CString& proposalId, MULTIPLAY::CMatchDraftTally& tally )
{
    GUCEF::vector< MULTIPLAY::CMatchDraftTally > all;
    table.P( player ).provider->GetDraftTallies( id, all );
    for ( size_t i=0; i<all.size(); ++i )
        if ( all[ i ].proposalId == proposalId )
        {
            tally = all[ i ];
            return true;
        }
    return false;
}

bool
WaitTally( CEMailTable& table, UInt32 player, const CORE::CString& id, const CORE::CString& proposalId, UInt32 votes )
{
    bool ok = false;
    MULTIPLAY::CMatchDraftTally tally;
    PUMP_WAIT( ok, GetTally( table, player, id, proposalId, tally ) && tally.yes + tally.no >= votes, 10000 );
    return ok;
}

bool
WaitState( CEMailTable& table, UInt32 player, const CORE::CString& id, const CORE::CString& proposalId, MULTIPLAY::TMatchDraftState state )
{
    bool ok = false;
    MULTIPLAY::CMatchDraftTally tally;
    PUMP_WAIT( ok, GetTally( table, player, id, proposalId, tally ) && tally.state == state, 10000 );
    return ok;
}

UInt32
ContactsOf( CEMailTable& table, UInt32 host, const CORE::CString& id )
{
    GUCEF::vector< MAIL::CEMailIntroContact > contacts;
    return table.P( host ).provider->GetIntroductions( id, contacts ) ? (UInt32) contacts.size() : 0;
}

UInt32
CountCodes( const GUCEF::vector< MAIL::CEMailMatchNotice >& notices, const char* code )
{
    UInt32 count = 0;
    for ( size_t i=0; i<notices.size(); ++i )
        if ( notices[ i ].code == code )
            ++count;
    return count;
}

/** What the application does when the draft is as the players want it: the signed record with the keys of the introduced, the first turn, CreateMatch */
bool
StartFromDraft( CEMailTable& table, UInt32 creator, const CORE::CString& matchId, MULTIPLAY::CMatchOperationResult& result )
{
    MAIL::CEMailIntroDraft draft;
    if ( !table.P( creator ).provider->GetIntroductionDraft( matchId, draft ) )
        return false;
    GUCEF::vector< MAIL::CEMailIntroContact > introduced;
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
    MULTIPLAY::CMatchRecord record( info );
    if ( !record.Sign( table.P( creator ).identity ) )
        return false;
    MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game|setup|start" );
    MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( record.info, table.P( creator ).identity, package, GUCEF_NULL, MULTIPLAY::MATCHSTATUS_ACTIVE, table.clock.nowMs );
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( !table.P( creator ).provider->CreateMatch( record, package, preview, future ) )
        return false;
    return table.Await( future, result );
}

/** The roster of the host with a content changed, signed by the host, for a revision */
MULTIPLAY::CMatchRoster
Altered( const MULTIPLAY::CMatchRoster& roster, UInt32 revision, const MULTIPLAY::CPlayerIdentity& signer, const char* name )
{
    MULTIPLAY::CMatchRoster other = roster;
    other.revision = revision;
    other.entries[ other.entries.size() - 1 ].displayName = name;
    other.Sign( signer );
    return other;
}

CORE::CString
RosterMail( CEMailTable& table, UInt32 from, UInt32 to, const CORE::CString& id, const MULTIPLAY::CMatchRoster& roster, const char* messageId )
{
    CORE::CDynamicBuffer bytes;
    if ( !MAIL::CEMailMatchCodec::SerializeToBytes( roster, bytes, 256 * 1024 ) )
        return CORE::CString::Empty;
    GUCEF::vector< CORE::CString > recipients;
    recipients.push_back( table.P( to ).address );
    return table.MailTextForData( table.P( from ).address.C_String(), recipients, id, roster.revision, MAIL::MAILKIND_ROSTER, bytes, MAIL::CEMailMatchCodec::ArmorLabelData, "roster", MAIL::MAILTRANSFER_ATTACHMENT, messageId );
}

CORE::CString
VoteMail( CEMailTable& table, UInt32 from, UInt32 to, const CORE::CString& id, const MULTIPLAY::CMatchVote& vote, const char* messageId )
{
    CORE::CDynamicBuffer bytes;
    if ( !MAIL::CEMailMatchCodec::SerializeToBytes( vote, bytes, 256 * 1024 ) )
        return CORE::CString::Empty;
    GUCEF::vector< CORE::CString > recipients;
    recipients.push_back( table.P( to ).address );
    return table.MailTextForData( table.P( from ).address.C_String(), recipients, id, 0, MAIL::MAILKIND_DRAFT_VOTE, bytes, MAIL::CEMailMatchCodec::ArmorLabelData, "vote", MAIL::MAILTRANSFER_ATTACHMENT, messageId );
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailDraftVoteTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: the table and the decisions before the record exists" );

    GUCEF_TESTFW_TESTCASE( "DraftVotes: the host publishes the table as the draft changes and every player holds the same signed table, with the keys of the others from it" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: the host publishes the table as the draft changes and every player holds the same signed table, with the keys of the others from it" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions options;
        options.seatsSettingKey = "seats";
        options.autoClockMs = 1;
        options.useIdle = false;
        ASSERT_TRUE( table.Setup( 4, "draftvotes-table", options ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 3 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // the host has the first roster at once: itself and three players who have not answered, under placeholders
        MULTIPLAY::CMatchRoster first;
        ASSERT_TRUE( table.P( 0 ).provider->GetDraftRoster( id, first ) && 1 == first.revision && 4 == first.entries.size() && 1 == first.CountVoters() );
        ASSERT_TRUE( MULTIPLAY::CMatchRosterEntry::IsPendingUserId( first.entries[ 1 ].userId ) && first.entries[ 1 ].publicKey.IsNULLOrEmpty() );

        const UInt32 accepting[] = { 3, 1, 2 };
        ASSERT_TRUE( IntroduceAll( table, id, accepting, 3 ) );
        for ( UInt32 p=0; p<4; ++p )
            ASSERT_TRUE( WaitVoters( table, p, id, 4 ) );

        // the same table on every machine, signed by the host, and the keys are in it
        MULTIPLAY::CMatchRoster host;
        ASSERT_TRUE( table.P( 0 ).provider->GetDraftRoster( id, host ) && host.revision >= 4 );
        CORE::CString reason;
        ASSERT_TRUE( host.Verify( table.P( 0 ).identity.publicKey, reason ) );
        for ( UInt32 p=1; p<4; ++p )
        {
            MULTIPLAY::CMatchRoster seen;
            bool same = false;
            PUMP_WAIT( same, table.P( p ).provider->GetDraftRoster( id, seen ) && seen.GetDigest() == host.GetDigest(), 10000 );
            ASSERT_TRUE( same );
            for ( UInt32 other=0; other<4; ++other )
            {
                const MULTIPLAY::CMatchRosterEntry* entry = seen.FindEntry( table.P( other ).identity.userId );
                ASSERT_TRUE( GUCEF_NULL != entry && entry->publicKey == table.P( other ).identity.publicKey && entry->contact == table.P( other ).address );
            }
            ASSERT_TRUE( table.CountMatchMails( p, "ROSTER" ) >= 1 );
            ASSERT_TRUE( CountCodes( table.P( p ).recorder.rosters, "ROSTER" ) >= 1 );
        }
        // Dave knows the key of Bob from the roster alone: the address book of a player holds the host and nobody else
        MAIL::CEMailAddressEntry known;
        ASSERT_FALSE( table.P( 3 ).provider->GetAddressBook().Find( table.P( 1 ).address, known ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: an invitee proposes a force start, another votes, the host carries it out and every player counts the same tally" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: an invitee proposes a force start, another votes, the host carries it out and every player counts the same tally" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions options;
        options.seatsSettingKey = "seats";
        options.autoClockMs = 1;
        options.useIdle = false;
        ASSERT_TRUE( table.Setup( 4, "draftvotes-force", options ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 3 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        const UInt32 accepting[] = { 1, 2 };
        ASSERT_TRUE( IntroduceAll( table, id, accepting, 2 ) );
        ASSERT_TRUE( WaitVoters( table, 0, id, 3 ) && WaitVoters( table, 1, id, 3 ) && WaitVoters( table, 2, id, 3 ) && WaitPending( table, 3, id ) );

        CORE::CString proposalId, why;
        ASSERT_TRUE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_FORCE_START, CORE::CString::Empty, proposalId, why ) );
        // everybody who was asked sees it arrive, Dave who has not answered too
        for ( UInt32 p=0; p<4; ++p )
            ASSERT_TRUE( WaitTally( table, p, id, proposalId, 0 ) && 1 == CountCodes( table.P( p ).recorder.draftMessages, "DRAFT_PROPOSAL" ) );
        MULTIPLAY::CMatchDraftTally tally;
        ASSERT_TRUE( GetTally( table, 2, id, proposalId, tally ) && 1 == tally.yes && 0 == tally.no && 2 == tally.needed && 3 == tally.eligible && MULTIPLAY::MATCHDRAFTSTATE_OPEN == tally.state );
        ASSERT_TRUE( 3 == ContactsOf( table, 0, id ) );          // nothing is done before the vote has passed

        ASSERT_TRUE( table.P( 2 ).provider->VoteInDraft( id, proposalId, true, why ) );
        // the host carries it out: the player who did not answer is out of the draft and the draft can start with the others
        bool applied = false;
        PUMP_WAIT( applied, 2 == ContactsOf( table, 0, id ) && 1 == table.P( 0 ).recorder.draftApplied.size(), 10000 );
        ASSERT_TRUE( applied );
        ASSERT_TRUE( "FORCE_START" == table.P( 0 ).recorder.draftApplied[ 0 ].code );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && summary.startable && summary.allIntroduced );
        // the number of seats is not lowered by a force start: the computer plays the seat
        MAIL::CEMailIntroDraft draft;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionDraft( id, draft ) && "4" == draft.settings[ "seats" ] );

        // the same tally everywhere, and the vote was seen by all as it came in
        for ( UInt32 p=0; p<3; ++p )
            ASSERT_TRUE( WaitState( table, p, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_APPLIED ) );
        for ( UInt32 p=0; p<4; ++p )
        {
            ASSERT_TRUE( WaitTally( table, p, id, proposalId, 2 ) );
            ASSERT_TRUE( GetTally( table, p, id, proposalId, tally ) );
            ASSERT_TRUE( 2 == tally.yes && 0 == tally.no && 2 == tally.needed && 3 == tally.eligible && MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome );
            ASSERT_TRUE( 2 == tally.yesVoters.size() && tally.yesVoters[ 0 ] == table.P( 1 ).identity.userId && tally.yesVoters[ 1 ] == table.P( 2 ).identity.userId && 1 == tally.notVoted.size() );
        }
        ASSERT_TRUE( 1 == CountCodes( table.P( 0 ).recorder.draftMessages, "DRAFT_VOTE" ) && 1 == CountCodes( table.P( 1 ).recorder.draftMessages, "DRAFT_VOTE" ) );
        // the roster of the host has no seat for the player who was replaced, and the others kept their answer
        MULTIPLAY::CMatchRoster roster;
        bool rosterBack = false;
        PUMP_WAIT( rosterBack, table.P( 1 ).provider->GetDraftRoster( id, roster ) && 3 == roster.entries.size() && 1 == roster.evidence.size(), 10000 );
        ASSERT_TRUE( rosterBack && 3 == roster.CountVoters() );
        ASSERT_TRUE( table.CountMatchMails( 3, "DRAFT_PROPOSAL" ) == 1 && table.CountMatchMails( 3, "DRAFT_VOTE" ) == 1 );
        // Dave is told, signed, that the invitation is withdrawn
        MAIL::CEMailInboundIntro::EStatus status = MAIL::CEMailInboundIntro::STATUS_PENDING;
        bool cancelled = false;
        PUMP_WAIT( cancelled, HasPending( table, 3, id, &status ) && MAIL::CEMailInboundIntro::STATUS_CANCELLED == status, 10000 );
        ASSERT_TRUE( cancelled );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: a kick by vote takes the player out and lowers the seats, the others keep their answer and join the game at once" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: a kick by vote takes the player out and lowers the seats, the others keep their answer and join the game at once" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        TESTFW::CLogTextCapture logs;
        CEMailTable table;
        SOptions options;
        options.seatsSettingKey = "seats";
        options.autoClockMs = 1;
        options.useIdle = false;
        ASSERT_TRUE( table.Setup( 4, "draftvotes-kick", options ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 3 );
        const UInt32 accepting[] = { 1, 2, 3 };
        ASSERT_TRUE( IntroduceAll( table, id, accepting, 3 ) );
        for ( UInt32 p=0; p<4; ++p )
            ASSERT_TRUE( WaitVoters( table, p, id, 4 ) );

        CORE::CString proposalId, why;
        ASSERT_TRUE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_KICK, table.P( 3 ).identity.userId, proposalId, why ) );
        ASSERT_TRUE( WaitTally( table, 2, id, proposalId, 0 ) && WaitTally( table, 0, id, proposalId, 0 ) );
        // one yes besides the proposer is not enough for four voters; the host, who is one of them, votes too
        ASSERT_TRUE( table.P( 2 ).provider->VoteInDraft( id, proposalId, true, why ) );
        ASSERT_TRUE( WaitTally( table, 0, id, proposalId, 2 ) && WaitTally( table, 3, id, proposalId, 2 ) );
        MULTIPLAY::CMatchDraftTally tally;
        ASSERT_TRUE( GetTally( table, 3, id, proposalId, tally ) && 2 == tally.yes && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == tally.outcome );
        ASSERT_TRUE( 3 == ContactsOf( table, 0, id ) );
        // the one who is about to be removed votes against; it does not help
        ASSERT_TRUE( table.P( 3 ).provider->VoteInDraft( id, proposalId, false, why ) );
        ASSERT_TRUE( WaitTally( table, 0, id, proposalId, 3 ) );
        ASSERT_TRUE( 3 == ContactsOf( table, 0, id ) );
        ASSERT_TRUE( table.P( 0 ).provider->VoteInDraft( id, proposalId, true, why ) );

        bool applied = false;
        PUMP_WAIT( applied, 2 == ContactsOf( table, 0, id ), 10000 );
        ASSERT_TRUE( applied );
        MAIL::CEMailIntroDraft draft;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionDraft( id, draft ) && "3" == draft.settings[ "seats" ] );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( table.P( 0 ).provider->GetIntroductionSummary( id, summary ) && 2 == summary.introduced && summary.startable );
        for ( UInt32 p=0; p<3; ++p )
            ASSERT_TRUE( WaitState( table, p, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_APPLIED ) );
        ASSERT_TRUE( GetTally( table, 1, id, proposalId, tally ) && 3 == tally.yes && 1 == tally.no && 4 == tally.eligible && 3 == tally.needed );
        MULTIPLAY::CMatchRoster roster;
        ASSERT_TRUE( table.P( 2 ).provider->GetDraftRoster( id, roster ) && 3 == roster.entries.size() && 3 == roster.CountVoters() );

        // the game: nobody has to answer again
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );
        for ( UInt32 p=0; p<3; ++p )
        {
            bool active = false;
            PUMP_WAIT( active, MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( p, id ), 10000 );
            ASSERT_TRUE( active );
        }
        ASSERT_TRUE( 1 == CountCodes( table.P( 1 ).recorder.intros, "INTRO_JOINED" ) && 1 == CountCodes( table.P( 2 ).recorder.intros, "INTRO_JOINED" ) );
        ASSERT_FALSE( table.WaitKnown( 3, id, 1500 ) );
        // the record is the table of the last roster
        for ( UInt32 p=0; p<3; ++p )
            ASSERT_TRUE( 0 == CountCodes( table.P( p ).recorder.rosters, "RECORD_DEVIATES" ) );
        ASSERT_FALSE( logs.Contains( table.P( 1 ).identity.publicKey ) || logs.Contains( "dave@mail.test" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: under VOTE the host does not remove a player alone: the direct removal is refused with the reason, a table that drops a player without a decision that passed is refused by every player, and a player who declined can be tidied away; under BOSS the host removes directly" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: under VOTE the host does not remove a player alone: the direct removal is refused with the reason, a table that drops a player without a decision that passed is refused by every player, and a player who declined can be tidied away; under BOSS the host removes directly" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        {
            CEMailTable table;
            SOptions options;
            options.seatsSettingKey = "seats";
            options.decisionPolicySettingKey = "decisionPolicy";
            options.autoClockMs = 1;
            options.useIdle = false;
            ASSERT_TRUE( table.Setup( 4, "draftvotes-direct-vote", options ) && table.StartAll() );
            const UInt32 invitees[] = { 1, 2, 3 };
            const CORE::CString id = BeginDraft( table, 0, invitees, 3, "VOTE" );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            const UInt32 accepting[] = { 1, 2 };
            ASSERT_TRUE( IntroduceAll( table, id, accepting, 2 ) );
            ASSERT_TRUE( WaitPending( table, 3, id ) && MAIL::MAILINTRORESULT_OK == table.P( 3 ).provider->DeclineIntroduction( id ) );
            for ( UInt32 p=0; p<3; ++p )
                ASSERT_TRUE( WaitVoters( table, p, id, 3 ) );

            // the direct operation of the host says that the table decides, and changes nothing
            CORE::CString reason;
            ASSERT_FALSE( table.P( 0 ).provider->RemoveIntroducedPlayer( id, table.P( 1 ).address, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "vote", false ) >= 0 );
            ASSERT_TRUE( 3 == ContactsOf( table, 0, id ) );
            ASSERT_FALSE( table.P( 0 ).provider->RemoveIntroducedPlayer( id, table.P( 1 ).address, reason ) );

            // a table of the host that takes Bob off without a decision: signed by the host, refused by Bob, Carol and Dave
            MULTIPLAY::CMatchRoster current;
            ASSERT_TRUE( table.P( 0 ).provider->GetDraftRoster( id, current ) );
            MULTIPLAY::CMatchRoster forged = current;
            forged.revision = current.revision + 5;
            for ( size_t e=0; e<forged.entries.size(); ++e )
                if ( forged.entries[ e ].userId == table.P( 1 ).identity.userId )
                {
                    forged.entries.erase( forged.entries.begin() + e );
                    break;
                }
            forged.appliedDigests.clear();
            forged.evidence.clear();
            ASSERT_TRUE( forged.Sign( table.P( 0 ).identity ) );
            for ( UInt32 p=1; p<3; ++p )
                ASSERT_TRUE( table.Inject( p, RosterMail( table, 0, p, id, forged, ( CORE::CString( "<forged-roster-" ) + CORE::UInt32ToString( p ) + "@crafted.test>" ).C_String() ) ) );
            table.Settle( 1000 );
            for ( UInt32 p=1; p<3; ++p )
            {
                MULTIPLAY::CMatchRoster seen;
                ASSERT_TRUE( table.P( p ).provider->GetDraftRoster( id, seen ) && seen.revision < forged.revision && GUCEF_NULL != seen.FindEntry( table.P( 1 ).identity.userId ) );
                bool refused = false;
                for ( size_t n=0; n<table.P( p ).recorder.rosters.size(); ++n )
                    refused = refused || ( table.P( p ).recorder.rosters[ n ].code == "ROSTER_REFUSED" && table.P( p ).recorder.rosters[ n ].description == "UNAUTHORIZED_REMOVAL" );
                ASSERT_TRUE( refused );
            }

            // Dave declined: the host may take that entry off the table, nobody wanted to play there
            bool declined = false;
            PUMP_WAIT( declined, 3 == ContactsOf( table, 0, id ), 4000 );
            ASSERT_TRUE( table.P( 0 ).provider->RemoveIntroducedPlayer( id, table.P( 3 ).address, reason ) && 2 == ContactsOf( table, 0, id ) );
            for ( UInt32 p=1; p<3; ++p )
            {
                bool tidy = false;
                MULTIPLAY::CMatchRoster seen;
                PUMP_WAIT( tidy, table.P( p ).provider->GetDraftRoster( id, seen ) && 3 == seen.entries.size(), 10000 );
                ASSERT_TRUE( tidy );
            }
        }
        {
            CEMailTable table;
            SOptions options;
            options.seatsSettingKey = "seats";
            options.decisionPolicySettingKey = "decisionPolicy";
            options.autoClockMs = 1;
            options.useIdle = false;
            ASSERT_TRUE( table.Setup( 4, "draftvotes-direct-boss", options ) && table.StartAll() );
            const UInt32 invitees[] = { 1, 2, 3 };
            const CORE::CString id = BeginDraft( table, 0, invitees, 3, "BOSS" );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            const UInt32 accepting[] = { 1, 2, 3 };
            ASSERT_TRUE( IntroduceAll( table, id, accepting, 3 ) );
            for ( UInt32 p=0; p<4; ++p )
                ASSERT_TRUE( WaitVoters( table, p, id, 4 ) );
            CORE::CString reason;
            ASSERT_TRUE( table.P( 0 ).provider->RemoveIntroducedPlayer( id, table.P( 3 ).address, reason ) && 2 == ContactsOf( table, 0, id ) );
            for ( UInt32 p=1; p<3; ++p )
            {
                bool tidy = false;
                MULTIPLAY::CMatchRoster seen;
                PUMP_WAIT( tidy, table.P( p ).provider->GetDraftRoster( id, seen ) && 3 == seen.entries.size(), 10000 );
                ASSERT_TRUE( tidy );
                for ( size_t n=0; n<table.P( p ).recorder.rosters.size(); ++n )
                    ASSERT_FALSE( table.P( p ).recorder.rosters[ n ].code == "ROSTER_REFUSED" );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: a host that does not carry out a passed decision is shown as ignoring it after the time it is given, and carrying it out afterwards is seen by all" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: a host that does not carry out a passed decision is shown as ignoring it after the time it is given, and carrying it out afterwards is seen by all" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions options;
        options.seatsSettingKey = "seats";
        options.autoClockMs = 1;
        options.useIdle = false;
        ASSERT_TRUE( table.Setup( 4, "draftvotes-ignore", options ) );
        for ( UInt32 p=0; p<4; ++p )
        {
            table.P( p ).settings.draftApplyGraceMs = 5000;
            table.P( p ).settings.applyDraftDecisions = 0 != p;      // the host does not carry out anything by itself
            ASSERT_TRUE( table.P( p ).provider->SetSettings( table.P( p ).settings ) );
        }
        ASSERT_TRUE( table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 3 );
        const UInt32 accepting[] = { 1, 2, 3 };
        ASSERT_TRUE( IntroduceAll( table, id, accepting, 3 ) );
        for ( UInt32 p=0; p<4; ++p )
            ASSERT_TRUE( WaitVoters( table, p, id, 4 ) );
        CORE::CString proposalId, why;
        ASSERT_TRUE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_KICK, table.P( 3 ).identity.userId, proposalId, why ) );
        ASSERT_TRUE( WaitTally( table, 2, id, proposalId, 0 ) );
        ASSERT_TRUE( table.P( 2 ).provider->VoteInDraft( id, proposalId, true, why ) );
        ASSERT_TRUE( WaitTally( table, 0, id, proposalId, 2 ) );
        ASSERT_TRUE( table.P( 0 ).provider->VoteInDraft( id, proposalId, true, why ) );
        ASSERT_TRUE( WaitState( table, 1, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_PASSED ) && WaitState( table, 2, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_PASSED ) );

        // the host does nothing: within the time it is given the others wait, after it they show that the host did not carry it out
        table.Advance( 2000 );
        MULTIPLAY::CMatchDraftTally tally;
        ASSERT_TRUE( GetTally( table, 1, id, proposalId, tally ) && MULTIPLAY::MATCHDRAFTSTATE_PASSED == tally.state );
        table.Advance( 6000 );
        for ( UInt32 p=1; p<4; ++p )
        {
            ASSERT_TRUE( GetTally( table, p, id, proposalId, tally ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_HOST_IGNORED == tally.state && MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome && !tally.appliedByHost );
        }
        ASSERT_TRUE( 3 == ContactsOf( table, 0, id ) && 0 == table.P( 0 ).recorder.draftApplied.size() );
        // they cannot force it: the draft is as it was
        MULTIPLAY::CMatchRoster roster;
        ASSERT_TRUE( table.P( 1 ).provider->GetDraftRoster( id, roster ) && 4 == roster.entries.size() );

        // the host carries it out after all
        ASSERT_TRUE( 1 == table.P( 0 ).provider->ApplyDraftDecisions( id ) );
        for ( UInt32 p=0; p<3; ++p )
        {
            const bool reached = WaitState( table, p, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_APPLIED );
            MULTIPLAY::CMatchDraftTally debug;
            GetTally( table, p, id, proposalId, debug );
            if ( !reached )
                MAIL_TEST_NOTE( "player " << p << " state " << MULTIPLAY::MatchDraftStateToString( debug.state ) << " applied " << debug.appliedByHost << " rosters " << table.P( p ).recorder.rosters.size() << " problems " << table.P( p ).recorder.problems.size() << " " << table.Diagnose( p ).C_String() << " roster mails " << table.CountMatchMails( p, "ROSTER" ) );
            ASSERT_TRUE( reached );
        }
        ASSERT_TRUE( 2 == ContactsOf( table, 0, id ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: two rosters of the host for one revision are shown as equivocation, a roster of a stranger and a vote of a stranger count for nothing" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: two rosters of the host for one revision are shown as equivocation, a roster of a stranger and a vote of a stranger count for nothing" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions plain;
        plain.autoClockMs = 1;
        plain.useIdle = false;
        ASSERT_TRUE( table.Setup( 4, "draftvotes-forged", plain ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 3 );
        const UInt32 accepting[] = { 1, 2, 3 };
        ASSERT_TRUE( IntroduceAll( table, id, accepting, 3 ) );
        for ( UInt32 p=0; p<4; ++p )
            ASSERT_TRUE( WaitVoters( table, p, id, 4 ) );
        MULTIPLAY::CMatchRoster current;
        ASSERT_TRUE( table.P( 0 ).provider->GetDraftRoster( id, current ) );
        MULTIPLAY::CMatchRoster seen;
        bool same = false;
        PUMP_WAIT( same, table.P( 1 ).provider->GetDraftRoster( id, seen ) && seen.GetDigest() == current.GetDigest(), 10000 );
        ASSERT_TRUE( same );

        // a vote of somebody who is not at the table
        CORE::CString proposalId, why;
        ASSERT_TRUE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_KICK, table.P( 3 ).identity.userId, proposalId, why ) );
        ASSERT_TRUE( WaitTally( table, 2, id, proposalId, 0 ) );
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
        MULTIPLAY::CMatchProposal proposal;
        MULTIPLAY::CMatchDraftBoard carol;
        ASSERT_TRUE( table.P( 2 ).provider->GetDraftBoard( id, carol ) && carol.GetProposal( proposalId, proposal ) );
        MULTIPLAY::CMatchVote vote;
        ASSERT_TRUE( vote.Sign( mallory, proposal, true, table.clock.nowMs ) );
        ASSERT_TRUE( table.Inject( 2, VoteMail( table, 0, 2, id, vote, "<vote-m@crafted.test>" ) ) );
        bool problem = false;
        PUMP_WAIT( problem, table.P( 2 ).recorder.problems.size() >= 1, 8000 );
        ASSERT_TRUE( problem );
        MULTIPLAY::CMatchDraftTally tally;
        ASSERT_TRUE( GetTally( table, 2, id, proposalId, tally ) && 1 == tally.yes && 0 == tally.no );

        // the host (a client that was changed) signs two tables for the next step and mails them both to Bob
        const MULTIPLAY::CMatchRoster twinA = Altered( current, current.revision + 1, table.P( 0 ).identity, "Dave A" );
        const MULTIPLAY::CMatchRoster twinB = Altered( current, current.revision + 1, table.P( 0 ).identity, "Dave B" );
        ASSERT_TRUE( table.Inject( 1, RosterMail( table, 0, 1, id, twinA, "<twin-a@crafted.test>" ) ) );
        bool taken = false;
        PUMP_WAIT( taken, table.P( 1 ).provider->GetDraftRoster( id, seen ) && seen.revision == twinA.revision, 8000 );
        ASSERT_TRUE( taken && seen.GetDigest() == twinA.GetDigest() );
        ASSERT_TRUE( table.Inject( 1, RosterMail( table, 0, 1, id, twinB, "<twin-b@crafted.test>" ) ) );
        bool shown = false;
        PUMP_WAIT( shown, 1 == CountCodes( table.P( 1 ).recorder.rosters, "ROSTER_EQUIVOCATION" ), 8000 );
        ASSERT_TRUE( shown );
        MULTIPLAY::CMatchDraftBoard board;
        ASSERT_TRUE( table.P( 1 ).provider->GetDraftBoard( id, board ) && board.IsHostEquivocating() );
        UInt32 revision = 0;
        CORE::CString first, second;
        ASSERT_TRUE( board.GetEquivocation( revision, first, second ) && revision == twinA.revision && first == twinA.GetDigest() && second == twinB.GetDigest() );
        ASSERT_TRUE( table.P( 1 ).provider->GetDraftRoster( id, seen ) && seen.GetDigest() == twinA.GetDigest() );
        // Carol did not get either of them
        ASSERT_TRUE( table.P( 2 ).provider->GetDraftBoard( id, board ) && !board.IsHostEquivocating() );

        // a table that says it is the host's and is signed by Dave
        MULTIPLAY::CMatchRoster stranger = current;
        stranger.revision = current.revision + 2;
        stranger.Sign( table.P( 3 ).identity );
        ASSERT_TRUE( table.Inject( 2, RosterMail( table, 3, 2, id, stranger, "<stranger@crafted.test>" ) ) );
        bool refused = false;
        PUMP_WAIT( refused, 1 == CountCodes( table.P( 2 ).recorder.rosters, "ROSTER_REFUSED" ), 8000 );
        ASSERT_TRUE( refused );
        MULTIPLAY::CMatchRoster kept;
        ASSERT_TRUE( table.P( 2 ).provider->GetDraftRoster( id, kept ) && kept.GetDigest() == current.GetDigest() );

    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: a player who chooses to watch before the record exists is listed as watching by the host, and watches in the record" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: a player who chooses to watch before the record exists is listed as watching by the host, and watches in the record" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions plain;
        plain.autoClockMs = 1;
        plain.useIdle = false;
        ASSERT_TRUE( table.Setup( 3, "draftvotes-watch", plain ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 2 );
        ASSERT_TRUE( IntroduceAll( table, id, invitees, 2 ) );
        for ( UInt32 p=0; p<3; ++p )
            ASSERT_TRUE( WaitVoters( table, p, id, 3 ) );

        // a player cannot ask for another player to watch, and the host does not watch
        CORE::CString proposalId, why;
        ASSERT_FALSE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, table.P( 2 ).identity.userId, proposalId, why ) );
        ASSERT_FALSE( table.P( 0 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, table.P( 0 ).identity.userId, proposalId, why ) );
        ASSERT_TRUE( table.P( 2 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, table.P( 2 ).identity.userId, proposalId, why ) );
        for ( UInt32 p=0; p<3; ++p )
        {
            bool watches = false;
            MULTIPLAY::CMatchRoster roster;
            PUMP_WAIT( watches, table.P( p ).provider->GetDraftRoster( id, roster ) && GUCEF_NULL != roster.FindEntry( table.P( 2 ).identity.userId ) && MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == roster.FindEntry( table.P( 2 ).identity.userId )->status, 10000 );
            ASSERT_TRUE( watches && 2 == roster.CountVoters() );
        }
        for ( UInt32 p=0; p<3; ++p )
            ASSERT_TRUE( WaitState( table, p, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_APPLIED ) );
        ASSERT_TRUE( 1 == table.P( 0 ).recorder.draftApplied.size() && "MOVE_TO_SPECTATOR" == table.P( 0 ).recorder.draftApplied[ 0 ].code );

        // the game is made with the player in it, and everybody has the player as a spectator
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( StartFromDraft( table, 0, id, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitKnown( 1, id ) && table.WaitKnown( 2, id ) );
        for ( UInt32 p=0; p<3; ++p )
        {
            bool spectator = false;
            PUMP_WAIT( spectator, MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == table.StatusOf( p, id, 2 ), 10000 );
            ASSERT_TRUE( spectator );
        }
        bool joined = false;
        PUMP_WAIT( joined, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 1 ), 10000 );
        ASSERT_TRUE( joined );
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > evidence;
        ASSERT_TRUE( table.P( 0 ).provider->GetPreStartEvidence( id, evidence ) && 1 == evidence.size() && MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR == evidence[ 0 ].proposal.kind );
        for ( UInt32 p=0; p<3; ++p )
            ASSERT_TRUE( 0 == CountCodes( table.P( p ).recorder.rosters, "RECORD_DEVIATES" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: the votes and the table survive a restart of a player's application and of the host's" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: the votes and the table survive a restart of a player's application and of the host's" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions plain;
        plain.autoClockMs = 1;
        plain.useIdle = false;
        ASSERT_TRUE( table.Setup( 4, "draftvotes-restart", plain ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2, 3 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 3 );
        const UInt32 accepting[] = { 1, 2, 3 };
        ASSERT_TRUE( IntroduceAll( table, id, accepting, 3 ) );
        for ( UInt32 p=0; p<4; ++p )
            ASSERT_TRUE( WaitVoters( table, p, id, 4 ) );
        CORE::CString proposalId, why;
        ASSERT_TRUE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_KICK, table.P( 3 ).identity.userId, proposalId, why ) );
        ASSERT_TRUE( WaitTally( table, 2, id, proposalId, 0 ) );
        ASSERT_TRUE( table.P( 2 ).provider->VoteInDraft( id, proposalId, true, why ) );
        ASSERT_TRUE( WaitTally( table, 0, id, proposalId, 2 ) && WaitTally( table, 1, id, proposalId, 2 ) );

        MULTIPLAY::CMatchRoster before;
        ASSERT_TRUE( table.P( 0 ).provider->GetDraftRoster( id, before ) );
        const UInt32 rostersBefore = table.CountMatchMails( 1, "ROSTER" );
        ASSERT_TRUE( table.Restart( 1 ) && table.Restart( 0 ) );
        ASSERT_TRUE( table.P( 1 ).provider->Start() && table.P( 0 ).provider->Start() );
        MULTIPLAY::CMatchDraftTally tally;
        for ( UInt32 p=0; p<2; ++p )
        {
            MULTIPLAY::CMatchRoster after;
            ASSERT_TRUE( table.P( p ).provider->GetDraftRoster( id, after ) && after.GetDigest() == before.GetDigest() );
            ASSERT_TRUE( GetTally( table, p, id, proposalId, tally ) && 2 == tally.yes && 4 == tally.eligible );
        }
        table.Settle( 1500 );
        // the host did not publish its table again, and the vote that follows is counted on the machines that started again
        ASSERT_TRUE( table.CountMatchMails( 1, "ROSTER" ) == rostersBefore );
        ASSERT_TRUE( table.P( 0 ).provider->VoteInDraft( id, proposalId, true, why ) );
        ASSERT_TRUE( WaitState( table, 1, id, proposalId, MULTIPLAY::MATCHDRAFTSTATE_APPLIED ) );
        ASSERT_TRUE( GetTally( table, 1, id, proposalId, tally ) && 3 == tally.yes );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DraftVotes: a player who proposes on an older table gets the table of the host again, once" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "DraftVotes: a player who proposes on an older table gets the table of the host again, once" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CEMailTable table;
        SOptions plain;
        plain.autoClockMs = 1;
        plain.useIdle = false;
        ASSERT_TRUE( table.Setup( 3, "draftvotes-heal", plain ) && table.StartAll() );
        const UInt32 invitees[] = { 1, 2 };
        const CORE::CString id = BeginDraft( table, 0, invitees, 2 );
        const UInt32 first[] = { 1 };
        ASSERT_TRUE( IntroduceAll( table, id, first, 1 ) );
        ASSERT_TRUE( WaitVoters( table, 0, id, 2 ) && WaitVoters( table, 1, id, 2 ) && WaitPending( table, 2, id ) );
        // the table that follows Carol's answer is lost on its way
        table.world.DropNext( 1, "X-Redux-Kind: ROSTER" );
        const UInt32 second[] = { 2 };
        ASSERT_TRUE( IntroduceAll( table, id, second, 1 ) );
        ASSERT_TRUE( WaitVoters( table, 0, id, 3 ) );
        table.Settle( 1000 );
        ASSERT_TRUE( 2 == VotersOf( table, 1, id ) );

        // Bob proposes on the table he has; the host answers with the table it has
        CORE::CString proposalId, why;
        MULTIPLAY::CMatchRoster host;
        ASSERT_TRUE( table.P( 0 ).provider->GetDraftRoster( id, host ) );
        ASSERT_TRUE( table.P( 1 ).provider->ProposeInDraft( id, MULTIPLAY::MATCHDECISION_FORCE_START, CORE::CString::Empty, proposalId, why ) );
        bool healed = false;
        PUMP_WAIT( healed, 3 == VotersOf( table, 1, id ), 10000 );
        ASSERT_TRUE( healed );
        MULTIPLAY::CMatchRoster bob;
        ASSERT_TRUE( table.P( 1 ).provider->GetDraftRoster( id, bob ) && bob.GetDigest() == host.GetDigest() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
