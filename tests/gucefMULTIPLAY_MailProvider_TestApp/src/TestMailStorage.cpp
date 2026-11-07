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

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#include "../../gucefMULTIPLAY_TestApp/include/TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#ifndef GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H
#include "TestMailWorld.h"
#define GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H
#endif /* GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "matchprovidermail_CMailFiles.h"
#include "matchprovidermail_CMailOutbox.h"
#include "matchprovidermail_CMailMatchState.h"
#include "TestMailPump.h"
#include "TestMailStorage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
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

CORE::CString
StorageDir( const char* name )
{GUCEF_TRACE;

    CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "mailtemp" ), "storage" );
    dir = CORE::CombinePath( dir, name );
    CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

UInt32
CountFiles( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    MAIL::CMailFiles::ListFiles( dir, names );
    return (UInt32) names.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
Text( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    return bytes.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

/** Records what the outbox says */
class CDeliveryRecorder : public CORE::CObserver
{
    public:

    GUCEF::vector< MAIL::CMailDeliveryEvent > events;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString n = "CDeliveryRecorder"; return n; }

    UInt32 Count( MAIL::TMailDeliveryState state ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<events.size(); ++i )
            if ( events[ i ].state == state )
                ++count;
        return count;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MAIL::CMailOutbox::DeliveryChangedEvent == eventId )
            events.push_back( *static_cast< const MAIL::CMailDeliveryEvent* >( eventData ) );
    }
};

/*-------------------------------------------------------------------------*/

/** An outbox of Alice, with a world of three mailboxes and a clock the test moves */
struct SOutboxFixture
{
    TESTFW::CTestMailWorld world;
    TESTFW::CTestMailBox* alice;
    TESTFW::CTestMailBox* bob;
    TESTFW::CTestMailBox* carol;
    TESTFW::CManualMatchClock clock;
    MAIL::CMailMatchProviderSettings settings;
    MAIL::CMailOutbox outbox;
    CDeliveryRecorder recorder;
    CORE::CString dir;
    CORE::CString matchId;
    UInt32 counter;

    SOutboxFixture( void ) : world(), alice( GUCEF_NULL ), bob( GUCEF_NULL ), carol( GUCEF_NULL ), clock(), settings(), outbox(), recorder(), dir(), matchId( MULTIPLAY::CMatchId::Generate() ), counter( 0 ) {}

    bool Setup( const char* name )
    {
        dir = StorageDir( name );
        alice = world.AddMailbox( "alice@mail.test", "alice", "alice-pw" );
        bob = world.AddMailbox( "bob@mail.test", "bob", "bob-pw" );
        carol = world.AddMailbox( "carol@mail.test", "carol", "carol-pw" );
        if ( !world.Start() )
            return false;
        settings.mailAddress = "alice@mail.test";
        settings.smtp.host = "127.0.0.1";
        settings.smtp.port = alice->smtp.GetPort();
        settings.smtp.tlsMode = COM::MAILTLS_NONE;
        settings.smtp.allowInsecureAuth = true;
        settings.smtp.userName = "alice";
        settings.smtp.password = "alice-pw";
        settings.smtp.connectTimeoutInMs = 5000;
        settings.smtp.commandTimeoutInMs = 5000;
        settings.outboxResendIntervalMs = 60000;
        settings.sendRetryInitialMs = 1000;
        settings.sendRetryMaxMs = 8000;
        settings.outboxMaxSendAttempts = 5;
        outbox.Configure( &settings, GUCEF_NULL, CORE::CombinePath( dir, "outbox" ) );
        recorder.SubscribeTo( &outbox );
        return true;
    }

    /** A mail of Alice to the others, as an outbox item that waits for them to confirm */
    CORE::CString QueueTurn( UInt32 revision, bool toBob, bool toCarol, UInt32* itemId = GUCEF_NULL )
    {
        MAIL::CMailOutgoing outgoing;
        outgoing.kind = MAIL::MAILKIND_TURN;
        outgoing.matchId = matchId;
        outgoing.revision = revision;
        outgoing.turn = revision;
        outgoing.title = "Storage";
        outgoing.from = COM::CMailAddress( "Alice", "alice@mail.test" );
        if ( toBob ) outgoing.to.push_back( COM::CMailAddress( "bob@mail.test" ) );
        if ( toCarol ) ( outgoing.to.empty() ? outgoing.to : outgoing.cc ).push_back( COM::CMailAddress( "carol@mail.test" ) );
        CORE::CDynamicBuffer data;
        data.Append( "payload", 7 );
        outgoing.data = &data;
        outgoing.dataLabel = MAIL::CMailMatchCodec::ArmorLabelTurn;
        outgoing.dataRevisionName = "r" + CORE::UInt32ToString( revision );
        COM::CMailMessage message;
        CORE::CString reason;
        if ( !MAIL::CMailMatchCodec::Compose( outgoing, message, reason ) )
            return CORE::CString::Empty;
        const CORE::CString messageId = "<storage-" + CORE::UInt32ToString( ++counter ) + "@mail.test>";
        message.messageId = messageId;
        COM::CMimeComposer composer;
        MAIL::CMailOutboxItem item;
        if ( !composer.Compose( message, item.message, reason ) )
            return CORE::CString::Empty;
        item.kind = MAIL::MAILKIND_TURN;
        item.matchId = matchId;
        item.revision = revision;
        item.messageId = messageId;
        item.envelopeFrom = "alice@mail.test";
        item.needsConfirmation = true;
        if ( toBob ) { item.recipients.push_back( "bob@mail.test" ); item.awaiting.push_back( "bob@mail.test" ); }
        if ( toCarol ) { item.recipients.push_back( "carol@mail.test" ); item.awaiting.push_back( "carol@mail.test" ); }
        const UInt32 id = outbox.Enqueue( item, clock.nowMs );
        if ( GUCEF_NULL != itemId )
            *itemId = id;
        return 0 == id ? CORE::CString::Empty : messageId;
    }

    /** Runs the outbox for the time, at the manual clock; the clock advances with it when moveClock is set */
    void Run( UInt32 ms, bool moveClock = false )
    {
        for ( UInt32 i=0; i<ms; ++i )
        {
            outbox.Update( clock.nowMs );
            PumpOnce();
            if ( moveClock )
                clock.AdvanceMs( 1 );
        }
    }

    bool RunUntilAccepted( UInt32 count, UInt32 timeoutMs )
    {
        bool ok = false;
        for ( UInt32 i=0; i<timeoutMs && !ok; ++i )
        {
            outbox.Update( clock.nowMs );
            PumpOnce();
            ok = world.GetAcceptedCount() >= count;
        }
        return ok;
    }
};

/*-------------------------------------------------------------------------*/

MAIL::CMailOutboxItem
MakeFullItem( void )
{GUCEF_TRACE;

    MAIL::CMailOutboxItem item;
    item.id = 42;
    item.kind = MAIL::MAILKIND_FINISH;
    item.matchId = MULTIPLAY::CMatchId::Generate();
    item.revision = 9;
    item.messageId = "<item@mail.test>";
    item.envelopeFrom = "alice@mail.test";
    item.recipients.push_back( "bob@mail.test" );
    item.recipients.push_back( "carol@mail.test" );
    item.awaiting.push_back( "carol@mail.test" );
    item.packageSha256 = "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff";
    item.needsConfirmation = true;
    item.state = MAIL::MAILDELIVERY_SENT;
    item.attempts = 3;
    item.createdAt = 1700000000000LL;
    item.nextAttemptAt = 1700000060000LL;
    item.lastSentAt = 1700000030000LL;
    item.message.Append( "From: a@b\r\n\r\nbody\r\n", 19 );
    return item;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailStorageTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: files, outbox and state" );

    GUCEF_TESTFW_TESTCASE( "Files: a file is written through a temporary file and a rename, read within a bound, and listed" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Files: a file is written through a temporary file and a rename, read within a bound, and listed" ), "not the case that was asked for" );
    try
    {
        const CORE::CString dir = StorageDir( "files" );
        const CORE::CString path = CORE::CombinePath( CORE::CombinePath( dir, "a" ), "file.bin" );
        CORE::CDynamicBuffer read;
        ASSERT_FALSE( MAIL::CMailFiles::ReadBounded( path, 1000, read ) );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( path, "first", 5 ) );
        ASSERT_TRUE( 1 == CountFiles( CORE::CombinePath( dir, "a" ) ) );
        ASSERT_TRUE( MAIL::CMailFiles::ReadBounded( path, 1000, read ) && "first" == Text( read ) );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( path, "second file", 11 ) );
        ASSERT_TRUE( 1 == CountFiles( CORE::CombinePath( dir, "a" ) ) );
        ASSERT_TRUE( MAIL::CMailFiles::ReadBounded( path, 1000, read ) && "second file" == Text( read ) );
        // larger than the bound is not read at all
        ASSERT_FALSE( MAIL::CMailFiles::ReadBounded( path, 10, read ) );
        ASSERT_TRUE( MAIL::CMailFiles::ReadBounded( path, 11, read ) );
        // an empty file
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( CORE::CombinePath( dir, "empty" ), GUCEF_NULL, 0 ) );
        ASSERT_TRUE( MAIL::CMailFiles::ReadBounded( CORE::CombinePath( dir, "empty" ), 10, read ) && 0 == read.GetDataSize() );

        CORE::CString::StringVector dirs;
        MAIL::CMailFiles::ListDirectories( dir, dirs );
        ASSERT_TRUE( 1 == dirs.size() && "a" == dirs[ 0 ] );
        CORE::CString::StringVector files;
        MAIL::CMailFiles::ListFiles( dir, files );
        ASSERT_TRUE( 1 == files.size() && "empty" == files[ 0 ] );
        MAIL::CMailFiles::ListFiles( CORE::CombinePath( dir, "none" ), files );
        ASSERT_TRUE( files.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Files: a record is stored as JSON and read back, a bound and damage are refused" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Files: a record is stored as JSON and read back, a bound and damage are refused" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        const CORE::CString dir = StorageDir( "records" );
        MULTIPLAY::CPlayerIdentity alice;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Alice", alice ) );
        GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids( 1, alice );
        MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "record", true );
        MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game" );
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, alice, package, GUCEF_NULL, MULTIPLAY::MATCHSTATUS_ACTIVE, 5 );

        const CORE::CString path = CORE::CombinePath( dir, "preview.json" );
        ASSERT_TRUE( MAIL::CMailFiles::WriteRecord( path, preview ) );
        ASSERT_TRUE( 1 == CountFiles( dir ) );
        MULTIPLAY::CMatchPreview read;
        ASSERT_TRUE( MAIL::CMailFiles::ReadRecord( path, read, 64 * 1024 ) && read.VerifySignature() && read.packageSha256 == preview.packageSha256 );
        MULTIPLAY::CMatchPreview tooSmall;
        ASSERT_FALSE( MAIL::CMailFiles::ReadRecord( path, tooSmall, 50 ) );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( CORE::CombinePath( dir, "junk.json" ), "{ not json", 10 ) );
        ASSERT_FALSE( MAIL::CMailFiles::ReadRecord( CORE::CombinePath( dir, "junk.json" ), tooSmall, 64 * 1024 ) );
        ASSERT_FALSE( MAIL::CMailFiles::ReadRecord( CORE::CombinePath( dir, "missing.json" ), tooSmall, 64 * 1024 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox item: the file layout round trips, and truncation, extra bytes and any damage never crash the reader" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox item: the file layout round trips, and truncation, extra bytes and any damage never crash the reader" ), "not the case that was asked for" );
    try
    {
        const MAIL::CMailOutboxItem item = MakeFullItem();
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( item.ToBytes( bytes ) );
        MAIL::CMailOutboxItem read;
        ASSERT_TRUE( read.FromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) );
        ASSERT_TRUE( read.id == item.id && read.kind == item.kind && read.matchId == item.matchId && read.revision == item.revision && read.messageId == item.messageId );
        ASSERT_TRUE( read.envelopeFrom == item.envelopeFrom && 2 == read.recipients.size() && 1 == read.awaiting.size() && read.awaiting[ 0 ] == "carol@mail.test" );
        ASSERT_TRUE( read.packageSha256 == item.packageSha256 && read.needsConfirmation && read.state == item.state && 3 == read.attempts );
        ASSERT_TRUE( read.createdAt == item.createdAt && read.nextAttemptAt == item.nextAttemptAt && read.lastSentAt == item.lastSentAt && Text( read.message ) == Text( item.message ) );

        // every shorter file is refused
        for ( UInt32 size=0; size<bytes.GetDataSize(); ++size )
        {
            MAIL::CMailOutboxItem partial;
            ASSERT_FALSE( partial.FromBytes( bytes.GetConstBufferPtr(), size ) );
        }
        // bytes behind the end are refused
        CORE::CDynamicBuffer longer = bytes;
        longer.Append( "x", 1 );
        MAIL::CMailOutboxItem extra;
        ASSERT_FALSE( extra.FromBytes( longer.GetConstBufferPtr(), longer.GetDataSize() ) );
        // damage: the reader answers, whatever it answers
        UInt32 state = 7;
        for ( int round=0; round<3000; ++round )
        {
            CORE::CDynamicBuffer damaged = bytes;
            UInt8* data = static_cast< UInt8* >( damaged.GetBufferPtr() );
            state = state * 1664525u + 1013904223u;
            const UInt32 flips = 1 + ( state >> 28 ) % 3;
            for ( UInt32 f=0; f<flips; ++f )
            {
                state = state * 1664525u + 1013904223u;
                data[ ( state >> 8 ) % damaged.GetDataSize() ] ^= (UInt8)( 1 << ( ( state >> 3 ) % 8 ) );
            }
            MAIL::CMailOutboxItem result;
            result.FromBytes( damaged.GetConstBufferPtr(), damaged.GetDataSize() );
        }
        // a recipient count of billions
        CORE::CDynamicBuffer huge;
        MULTIPLAY::CMatchBinaryWriter writer( huge );
        writer.WriteRaw( bytes.GetConstBufferPtr(), 12 );
        writer.WriteUInt32( 0xFFFFFFFFu );
        MAIL::CMailOutboxItem hugeResult;
        ASSERT_FALSE( hugeResult.FromBytes( huge.GetConstBufferPtr(), huge.GetDataSize() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Match state: everything a match is remembered by is written and read back, and a damaged file is refused" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Match state: everything a match is remembered by is written and read back, and a damaged file is refused" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
        for ( int i=0; i<3; ++i )
        {
            MULTIPLAY::CPlayerIdentity identity;
            ASSERT_TRUE( TESTFW::MakeTestIdentity( i == 0 ? "Alice" : ( i == 1 ? "Bob" : "Carol" ), identity ) );
            ids.push_back( identity );
        }
        MAIL::CMailMatchState state;
        state.info = TESTFW::MakeTestMatchInfo( ids, "state", false );
        state.record = MULTIPLAY::CMatchRecord( state.info );
        ASSERT_TRUE( state.record.Sign( ids[ 0 ] ) );
        state.hasRecord = true;
        MULTIPLAY::CMatchAcceptance acceptance;
        ASSERT_TRUE( acceptance.Sign( ids[ 1 ], state.info, true ) );
        state.acceptances.push_back( acceptance );
        MAIL::CMailSignedStatement resign;
        resign.matchId = state.info.matchId;
        resign.statementKind = MAIL::CMailSignedStatement::KindResign;
        ASSERT_TRUE( resign.Sign( ids[ 2 ] ) );
        state.resigns.push_back( resign );
        MULTIPLAY::CMatchProposal proposal;
        proposal.matchId = state.info.matchId;
        proposal.kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
        proposal.targetUserId = ids[ 2 ].userId;
        proposal.createdAt = 1700000000000LL;
        ASSERT_TRUE( proposal.Sign( ids[ 0 ] ) );
        state.proposals.push_back( proposal );
        MULTIPLAY::CMatchVote vote;
        ASSERT_TRUE( vote.Sign( ids[ 1 ], proposal, true, 1700000001000LL ) );
        state.votes.push_back( vote );
        state.appliedDecisions.push_back( proposal.GetDigest() );
        state.trustedRevision = 4;
        state.shaByRevision[ 3 ] = "aa";
        state.shaByRevision[ 4 ] = "bb";
        MAIL::CMailRefusal refusal;
        refusal.revision = 4; refusal.sha256 = "cc"; refusal.code = "NOT_ON_TURN"; refusal.reason = "The signer was not on turn"; refusal.at = 55; refusal.file = "refused-1.rdxturn";
        state.refusals.push_back( refusal );
        MAIL::CMailEquivocation equivocation;
        equivocation.revision = 3; equivocation.issuerKey = ids[ 1 ].publicKey; equivocation.issuerUserId = ids[ 1 ].userId; equivocation.firstSha256 = "aa"; equivocation.secondSha256 = "dd"; equivocation.file = "refused-2.rdxturn";
        state.equivocations.push_back( equivocation );
        state.lastMessageId = "<last@mail.test>";
        state.references.push_back( "<first@mail.test>" );
        state.references.push_back( "<last@mail.test>" );
        state.nudgesSent[ ids[ 1 ].userId ] = 1700000002000LL;
        state.nudgesReceived[ ids[ 0 ].userId ] = 1700000003000LL;
        state.resendRequested[ 2 ] = 1700000004000LL;
        state.resendAnswered[ "2:bob@mail.test" ] = 1700000005000LL;
        state.deliveredRevisions.insert( 1 );
        state.deliveredRevisions.insert( 3 );
        state.lastActivity = 1700000006000LL;
        state.cursor = "1000:7";

        const CORE::CString path = CORE::CombinePath( StorageDir( "state" ), "state.json" );
        ASSERT_TRUE( MAIL::CMailFiles::WriteRecord( path, state ) );
        MAIL::CMailMatchState read;
        ASSERT_TRUE( MAIL::CMailFiles::ReadRecord( path, read, 4 * 1024 * 1024 ) );
        CORE::CString why;
        ASSERT_TRUE( read.hasRecord && read.record.Verify( why ) && read.info.matchId == state.info.matchId && 3 == read.info.participants.size() );
        ASSERT_TRUE( 1 == read.acceptances.size() && read.acceptances[ 0 ].Verify( read.info, why ) );
        ASSERT_TRUE( 1 == read.resigns.size() && read.resigns[ 0 ].Verify( read.info, why ) );
        ASSERT_TRUE( 1 == read.proposals.size() && read.proposals[ 0 ].VerifySignature( read.info ) && 1 == read.votes.size() && read.votes[ 0 ].VerifySignature( read.info ) );
        ASSERT_TRUE( 1 == read.appliedDecisions.size() && read.appliedDecisions[ 0 ] == proposal.GetDigest() );
        ASSERT_TRUE( 4 == read.trustedRevision && 2 == read.shaByRevision.size() && read.shaByRevision[ 4 ] == "bb" );
        ASSERT_TRUE( 1 == read.refusals.size() && read.refusals[ 0 ].code == "NOT_ON_TURN" && read.refusals[ 0 ].file == "refused-1.rdxturn" && 55 == read.refusals[ 0 ].at );
        ASSERT_TRUE( 1 == read.equivocations.size() && read.equivocations[ 0 ].secondSha256 == "dd" && read.equivocations[ 0 ].issuerUserId == ids[ 1 ].userId );
        ASSERT_TRUE( read.lastMessageId == "<last@mail.test>" && 2 == read.references.size() && read.cursor == "1000:7" && read.lastActivity == state.lastActivity );
        ASSERT_TRUE( read.nudgesSent[ ids[ 1 ].userId ] == 1700000002000LL && read.nudgesReceived[ ids[ 0 ].userId ] == 1700000003000LL );
        ASSERT_TRUE( read.resendRequested[ 2 ] == 1700000004000LL && read.resendAnswered[ "2:bob@mail.test" ] == 1700000005000LL );
        ASSERT_TRUE( 2 == read.deliveredRevisions.size() && read.deliveredRevisions.count( 3 ) == 1 );

        // not a state
        MAIL::CMailMatchState junk;
        CORE::CDataNode empty( "Nothing" );
        ASSERT_FALSE( junk.Deserialize( empty, CORE::CDataNodeSerializableSettings() ) && junk.hasRecord );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: a mail is a file, goes to the mail server of the player, waits for the recipients to confirm and ends when all did" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: a mail is a file, goes to the mail server of the player, waits for the recipients to confirm and ends when all did" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-send" ) );
        UInt32 itemId = 0;
        const CORE::CString messageId = f.QueueTurn( 2, true, true, &itemId );
        ASSERT_TRUE( !messageId.IsNULLOrEmpty() && 0 != itemId );
        ASSERT_TRUE( 1 == CountFiles( CORE::CombinePath( f.dir, "outbox" ) ) && 1 == f.outbox.GetItemCount() );
        ASSERT_TRUE( MAIL::MAILDELIVERY_QUEUED == f.outbox.GetState( f.matchId, 2 ) );

        ASSERT_TRUE( f.RunUntilAccepted( 1, 8000 ) );
        f.Run( 100 );
        ASSERT_TRUE( MAIL::MAILDELIVERY_SENT == f.outbox.GetState( f.matchId, 2 ) && 1 == f.recorder.Count( MAIL::MAILDELIVERY_SENT ) );
        GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = f.alice->smtp.GetMessages();
        ASSERT_TRUE( 1 == received.size() && 2 == received[ 0 ].recipients.size() && "alice@mail.test" == received[ 0 ].mailFrom );
        ASSERT_TRUE( received[ 0 ].data.HasSubstr( messageId ) >= 0 );
        ASSERT_TRUE( 1 == f.bob->imap.GetMessages().size() && 1 == f.carol->imap.GetMessages().size() );
        // it is not sent again before the quiet period has passed
        f.Run( 300, true );
        ASSERT_TRUE( 1 == f.alice->smtp.GetMessages().size() );
        // the item stays until the recipients confirm: it is still a file
        ASSERT_TRUE( 1 == CountFiles( CORE::CombinePath( f.dir, "outbox" ) ) && 1 == f.outbox.GetUnconfirmedCount() );

        ASSERT_TRUE( 0 == f.outbox.Acknowledge( f.matchId, "bob@mail.test", 2, false ) );
        ASSERT_TRUE( MAIL::MAILDELIVERY_SENT == f.outbox.GetState( f.matchId, 2 ) );
        // the same recipient again, and a stranger, change nothing
        ASSERT_TRUE( 0 == f.outbox.Acknowledge( f.matchId, "BOB@mail.test", 2, false ) && 0 == f.outbox.Acknowledge( f.matchId, "mallory@mail.test", 2, false ) );
        // a confirmation of an older revision does not confirm a newer mail
        ASSERT_TRUE( 0 == f.outbox.Acknowledge( f.matchId, "carol@mail.test", 1, false ) );
        ASSERT_TRUE( 1 == f.outbox.Acknowledge( f.matchId, "Carol@Mail.Test", 3, false ) );
        ASSERT_TRUE( MAIL::MAILDELIVERY_NONE == f.outbox.GetState( f.matchId, 2 ) && 0 == f.outbox.GetItemCount() && 0 == CountFiles( CORE::CombinePath( f.dir, "outbox" ) ) );
        ASSERT_TRUE( 1 == f.recorder.Count( MAIL::MAILDELIVERY_DELIVERED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: a temporary failure keeps the mail and tries again later, for the recipients that were not served" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: a temporary failure keeps the mail and tries again later, for the recipients that were not served" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-temp" ) );
        f.alice->smtp.tempRejectRecipients.insert( "bob@mail.test" );
        ASSERT_FALSE( f.QueueTurn( 2, true, true ).IsNULLOrEmpty() );
        ASSERT_TRUE( f.RunUntilAccepted( 1, 8000 ) );
        f.Run( 200 );
        // Carol was served, Bob is still to do
        GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = f.alice->smtp.GetMessages();
        ASSERT_TRUE( 1 == received.size() && 1 == received[ 0 ].recipients.size() && "carol@mail.test" == received[ 0 ].recipients[ 0 ] );
        ASSERT_TRUE( 1 == f.carol->imap.GetMessages().size() && 0 == f.bob->imap.GetMessages().size() );
        ASSERT_TRUE( MAIL::MAILDELIVERY_QUEUED == f.outbox.GetState( f.matchId, 2 ) );
        // not before the wait has passed
        f.Run( 300 );
        ASSERT_TRUE( 1 == f.alice->smtp.GetMessages().size() );
        f.alice->smtp.tempRejectRecipients.clear();
        f.clock.AdvanceMs( 2500 );
        ASSERT_TRUE( f.RunUntilAccepted( 2, 8000 ) );
        f.Run( 100 );
        received = f.alice->smtp.GetMessages();
        ASSERT_TRUE( 2 == received.size() && 1 == received[ 1 ].recipients.size() && "bob@mail.test" == received[ 1 ].recipients[ 0 ] );
        ASSERT_TRUE( 1 == f.bob->imap.GetMessages().size() && MAIL::MAILDELIVERY_SENT == f.outbox.GetState( f.matchId, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: the wait after a temporary failure grows and has a limit" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: the wait after a temporary failure grows and has a limit" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-backoff" ) );
        f.alice->smtp.tempRejectRecipients.insert( "bob@mail.test" );
        UInt32 itemId = 0;
        ASSERT_FALSE( f.QueueTurn( 2, true, false, &itemId ).IsNULLOrEmpty() );
        UInt32 attempts = 0;
        Int64 lastGap = 0;
        // the clock moves in steps of a second; the mail server is asked again when the wait has passed
        Int64 lastAttemptAt = f.clock.nowMs;
        UInt32 previousAttempts = 0;
        GUCEF::vector< Int64 > gaps;
        for ( int second=0; second<40 && gaps.size() < 4; ++second )
        {
            f.Run( 60 );
            f.outbox.GetState( f.matchId, 2, &attempts );
            if ( attempts > previousAttempts )
            {
                gaps.push_back( f.clock.nowMs - lastAttemptAt );
                lastAttemptAt = f.clock.nowMs;
                previousAttempts = attempts;
            }
            f.clock.AdvanceMs( 1000 );
        }
        ASSERT_TRUE( gaps.size() >= 4 );
        // 1 s, 2 s, 4 s, then not more than 8 s
        ASSERT_TRUE( gaps[ 1 ] >= gaps[ 0 ] && gaps[ 2 ] >= gaps[ 1 ] && gaps[ 2 ] > gaps[ 0 ] );
        for ( size_t i=0; i<gaps.size(); ++i )
            ASSERT_TRUE( gaps[ i ] <= 9000 );
        lastGap = gaps[ gaps.size() - 1 ];
        ASSERT_TRUE( lastGap > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: a refusal for good is a visible failure without addresses in it, and the mail is kept for a resend" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: a refusal for good is a visible failure without addresses in it, and the mail is kept for a resend" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-permanent" ) );
        f.alice->smtp.rejectRecipients.insert( "bob@mail.test" );
        ASSERT_FALSE( f.QueueTurn( 2, true, false ).IsNULLOrEmpty() );
        bool failed = false;
        for ( UInt32 i=0; i<8000 && !failed; ++i )
        {
            f.Run( 1 );
            failed = f.recorder.Count( MAIL::MAILDELIVERY_FAILED ) > 0;
        }
        ASSERT_TRUE( failed );
        ASSERT_TRUE( MAIL::MAILDELIVERY_FAILED == f.outbox.GetState( f.matchId, 2 ) );
        for ( size_t i=0; i<f.recorder.events.size(); ++i )
            ASSERT_TRUE( f.recorder.events[ i ].description.HasSubstr( "bob@" ) < 0 && f.recorder.events[ i ].description.HasSubstr( "@mail.test" ) < 0 );
        // no more tries by itself
        f.alice->smtp.rejectRecipients.clear();
        f.clock.AdvanceMs( 10 * 60 * 1000 );
        f.Run( 300 );
        ASSERT_TRUE( 0 == f.alice->smtp.GetMessages().size() );
        // the player asks for it
        ASSERT_TRUE( 1 == f.outbox.ResendNow( f.matchId, 2, f.clock.nowMs ) );
        ASSERT_TRUE( f.RunUntilAccepted( 1, 8000 ) );
        f.Run( 100 );
        ASSERT_TRUE( 1 == f.bob->imap.GetMessages().size() && MAIL::MAILDELIVERY_SENT == f.outbox.GetState( f.matchId, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: a mail that nobody confirmed is sent again after the quiet period, to those who did not confirm, with the same Message-ID, up to a limit" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: a mail that nobody confirmed is sent again after the quiet period, to those who did not confirm, with the same Message-ID, up to a limit" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-resend" ) );
        const CORE::CString messageId = f.QueueTurn( 2, true, true );
        ASSERT_TRUE( f.RunUntilAccepted( 1, 8000 ) );
        f.Run( 100 );
        ASSERT_TRUE( 0 == f.outbox.Acknowledge( f.matchId, "bob@mail.test", 2, false ) );

        f.clock.AdvanceMs( 30000 );
        f.Run( 300 );
        ASSERT_TRUE( 1 == f.alice->smtp.GetMessages().size() );
        f.clock.AdvanceMs( 31000 );
        ASSERT_TRUE( f.RunUntilAccepted( 2, 8000 ) );
        f.Run( 100 );
        GUCEF::vector< TESTFW::CTestSmtpServer::TReceived > received = f.alice->smtp.GetMessages();
        ASSERT_TRUE( 2 == received.size() && 1 == received[ 1 ].recipients.size() && "carol@mail.test" == received[ 1 ].recipients[ 0 ] );
        ASSERT_TRUE( received[ 1 ].data.HasSubstr( messageId ) >= 0 );
        UInt32 attempts = 0;
        f.outbox.GetState( f.matchId, 2, &attempts );
        ASSERT_TRUE( 2 == attempts );

        // the limit of sends is 5: after that the mail stays as it is
        for ( int round=0; round<8; ++round )
        {
            f.clock.AdvanceMs( 61000 );
            f.Run( 400 );
        }
        f.outbox.GetState( f.matchId, 2, &attempts );
        ASSERT_TRUE( 5 == attempts && 5 == f.alice->smtp.GetMessages().size() );
        ASSERT_TRUE( MAIL::MAILDELIVERY_SENT == f.outbox.GetState( f.matchId, 2 ) && 1 == f.outbox.GetUnconfirmedCount() );

        // a resend on request does not wait for a quiet period
        ASSERT_TRUE( 1 == f.outbox.ResendNow( f.matchId, 2, f.clock.nowMs ) );
        ASSERT_TRUE( f.RunUntilAccepted( 6, 8000 ) );
        ASSERT_TRUE( 0 == f.outbox.ResendNow( f.matchId, 77, f.clock.nowMs ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: the mails survive a restart, a damaged file is skipped, and the files of a match can be removed" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: the mails survive a restart, a damaged file is skipped, and the files of a match can be removed" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-restart" ) );
        f.alice->smtp.tempRejectRecipients.insert( "bob@mail.test" );
        ASSERT_FALSE( f.QueueTurn( 2, true, false ).IsNULLOrEmpty() );
        ASSERT_FALSE( f.QueueTurn( 3, true, false ).IsNULLOrEmpty() );
        f.Run( 300 );
        f.outbox.Shutdown();
        const CORE::CString outDir = CORE::CombinePath( f.dir, "outbox" );
        ASSERT_TRUE( 2 == CountFiles( outDir ) );
        ASSERT_TRUE( MAIL::CMailFiles::WriteAtomic( CORE::CombinePath( outDir, "garbage.rdxout" ), "this is not an item", 19 ) );

        MAIL::CMailOutbox second;
        second.Configure( &f.settings, GUCEF_NULL, outDir );
        UInt32 skipped = 0;
        ASSERT_TRUE( second.Load( &skipped ) );
        ASSERT_TRUE( 2 == second.GetItemCount() && 1 == skipped );
        ASSERT_TRUE( MAIL::MAILDELIVERY_QUEUED == second.GetState( f.matchId, 2 ) && MAIL::MAILDELIVERY_QUEUED == second.GetState( f.matchId, 3 ) );
        // the damaged file was left alone
        ASSERT_TRUE( 3 == CountFiles( outDir ) );

        f.alice->smtp.tempRejectRecipients.clear();
        f.clock.AdvanceMs( 20000 );
        for ( UInt32 i=0; i<8000 && f.world.GetAcceptedCount() < 2; ++i )
        {
            second.Update( f.clock.nowMs );
            PumpOnce();
        }
        ASSERT_TRUE( 2 == f.world.GetAcceptedCount() );
        second.RemoveMatch( f.matchId );
        ASSERT_TRUE( 0 == second.GetItemCount() && 1 == CountFiles( outDir ) );
        second.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Outbox: a mail server that cannot be reached keeps the mail and says so without addresses" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Outbox: a mail server that cannot be reached keeps the mail and says so without addresses" ), "not the case that was asked for" );
    try
    {
        SOutboxFixture f;
        ASSERT_TRUE( f.Setup( "outbox-unreachable" ) );
        f.settings.smtp.port = 1;
        ASSERT_FALSE( f.QueueTurn( 2, true, false ).IsNULLOrEmpty() );
        f.Run( 1500 );
        ASSERT_TRUE( MAIL::MAILDELIVERY_QUEUED == f.outbox.GetState( f.matchId, 2 ) && f.recorder.Count( MAIL::MAILDELIVERY_QUEUED ) >= 1 );
        UInt32 attempts = 0;
        f.outbox.GetState( f.matchId, 2, &attempts );
        ASSERT_TRUE( attempts >= 1 );
        for ( size_t i=0; i<f.recorder.events.size(); ++i )
            ASSERT_TRUE( f.recorder.events[ i ].description.HasSubstr( "@" ) < 0 );
        // the server is back
        f.settings.smtp.port = f.alice->smtp.GetPort();
        f.clock.AdvanceMs( 30000 );
        ASSERT_TRUE( f.RunUntilAccepted( 1, 8000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
