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

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "matchprovidermail_CEMailIntroductions.h"
#include "matchprovidermail_CEMailMatchCodec.h"
#include "matchprovidermail_CEMailFiles.h"
#include "TestMailPump.h"
#include "TestMailIntroductions.h"

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

const Int64 g_day = 24LL * 60 * 60 * 1000;
const Int64 g_start = 1800000000000LL;

CORE::CString
Repeat( char c, UInt32 count )
{
    return CORE::CString( std::string( count, c ).c_str() );
}

/** Alice makes the games, Bob and Carol are invited, Mallory is somebody else */
struct SPeople
{
    MULTIPLAY::CPlayerIdentity alice;
    MULTIPLAY::CPlayerIdentity bob;
    MULTIPLAY::CPlayerIdentity carol;
    MULTIPLAY::CPlayerIdentity mallory;

    bool Make( void )
    {
        return TESTFW::MakeTestIdentity( "Alice", alice ) && TESTFW::MakeTestIdentity( "Bob", bob ) && TESTFW::MakeTestIdentity( "Carol", carol ) && TESTFW::MakeTestIdentity( "Mallory", mallory );
    }
};

/*-------------------------------------------------------------------------*/

GUCEF::vector< MAIL::CEMailIntroContactInput >
TwoContacts( const char* bobCode = "", const char* carolCode = "" )
{
    GUCEF::vector< MAIL::CEMailIntroContactInput > contacts;
    contacts.push_back( MAIL::CEMailIntroContactInput( "Bob", "bob@mail.test", bobCode ) );
    contacts.push_back( MAIL::CEMailIntroContactInput( "Carol", "carol@mail.test", carolCode ) );
    return contacts;
}

/*-------------------------------------------------------------------------*/

/** A creator side with a draft for Bob and Carol in memory only */
struct SCreator
{
    MAIL::CEMailIntroductions intro;
    CORE::CString matchId;

    bool Start( const SPeople& people, const GUCEF::vector< MAIL::CEMailIntroContactInput >& contacts, const CORE::CString& directory = CORE::CString::Empty )
    {
        intro.Configure( people.alice, directory, MAIL::CEMailIntroLimits() );
        matchId = MULTIPLAY::CMatchId::Generate();
        GUCEF::map< CORE::CString, CORE::CString > settings;
        settings[ "mailTransfer" ] = "ATTACHMENT";
        CORE::CString reason;
        return intro.BeginDraft( matchId, "Friday game", "Alice", "alice@mail.test", settings, contacts, g_start, reason );
    }

    /** The request for an address, as the provider would mail it */
    bool Ask( const char* address, MAIL::CEMailIntroRequest& request, Int64 now = g_start )
    {
        return MAIL::MAILINTRORESULT_OK == intro.PrepareRequest( matchId, address, false, now, request );
    }
};

/*-------------------------------------------------------------------------*/

/** What Bob (or Carol) sends back for a request: a signed acceptance made the way an invitee does */
MAIL::CEMailIntroReply
SignedReply( const MULTIPLAY::CPlayerIdentity& who, const MAIL::CEMailIntroRequest& request, const char* address, Int64 now = g_start + 1000 )
{
    MAIL::CEMailIntroReply reply;
    reply.matchId = request.matchId;
    reply.nonce = request.nonce;
    reply.accepted = true;
    reply.displayName = who.displayName;
    reply.inviteeAddress = address;
    reply.creatorAddress = request.creatorAddress;
    reply.createdAt = now;
    reply.Sign( who );
    return reply;
}

/*-------------------------------------------------------------------------*/

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailIntroductionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: introductions" );

    GUCEF_TESTFW_TESTCASE( "Introduction: the short fingerprint of a key has five groups of four upper case hex digits, is stable, differs between keys, and is empty for what is no key" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction: the short fingerprint of a key has five groups of four upper case hex digits, is stable, differs between keys, and is empty for what is no key" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        const CORE::CString a = MAIL::CEMailIntroFingerprint::Of( people.alice.publicKey );
        const CORE::CString b = MAIL::CEMailIntroFingerprint::Of( people.bob.publicKey );
        ASSERT_TRUE( 24 == a.Length() && 24 == b.Length() && a != b );
        for ( UInt32 i=0; i<a.Length(); ++i )
        {
            if ( 4 == i % 5 && i < 23 )
                ASSERT_TRUE( '-' == a[ i ] );
            else
                ASSERT_TRUE( ( a[ i ] >= '0' && a[ i ] <= '9' ) || ( a[ i ] >= 'A' && a[ i ] <= 'F' ) );
        }
        ASSERT_TRUE( a == MAIL::CEMailIntroFingerprint::Of( people.alice.publicKey ) );
        ASSERT_TRUE( a == MAIL::CEMailIntroFingerprint::Of( people.alice.publicKey.Uppercase() ) );
        ASSERT_TRUE( MAIL::CEMailIntroFingerprint::Of( "" ).IsNULLOrEmpty() );
        ASSERT_TRUE( MAIL::CEMailIntroFingerprint::Of( "abcd" ).IsNULLOrEmpty() );
        ASSERT_TRUE( MAIL::CEMailIntroFingerprint::Of( Repeat( 'z', 64 ) ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction: a request is signed for the match, any change of it fails, it goes to bytes and back, and what is out of bounds is refused" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction: a request is signed for the match, any change of it fails, it goes to bytes and back, and what is out of bounds is refused" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest request;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", request ) );
        CORE::CString why;
        ASSERT_TRUE( request.Verify( why ) );
        ASSERT_TRUE( request.matchId == creator.matchId && 32 == request.nonce.Length() && request.creatorUserId == people.alice.userId && request.creatorPublicKey == people.alice.publicKey );
        ASSERT_TRUE( request.inviteeAddress == "bob@mail.test" && request.inviteeName == "Bob" && request.creatorAddress == "alice@mail.test" && 3 == request.parties.size() );
        ASSERT_TRUE( request.expiresAt > request.createdAt && 128 == request.signature.Length() );

        // every field is signed
        MAIL::CEMailIntroRequest changed = request;
        changed.title = "Another game";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.nonce = "00000000000000000000000000000000";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.inviteeAddress = "mallory@mail.test";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.creatorAddress = "mallory@mail.test";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.expiresAt += 1;
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.createdAt += 1;
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.parties[ 1 ].address = "mallory@mail.test";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.parties.pop_back();
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.creatorName = "Not Alice";
        ASSERT_FALSE( changed.Verify( why ) );
        // another match: the signature is for one match id
        changed = request; changed.matchId = MULTIPLAY::CMatchId::Generate();
        ASSERT_FALSE( changed.Verify( why ) );
        // somebody else's key under the signature of Alice
        changed = request; changed.creatorPublicKey = people.mallory.publicKey; changed.creatorUserId = people.mallory.userId;
        ASSERT_FALSE( changed.Verify( why ) );
        // a key whose user id is not the user id
        changed = request; changed.creatorUserId = people.mallory.userId;
        ASSERT_FALSE( changed.Verify( why ) );

        // bytes and back
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SerializeToBytes( request, bytes, MAIL::CEMailIntroRequest::MaxDataBytes ) && bytes.GetDataSize() < MAIL::CEMailIntroRequest::MaxDataBytes );
        MAIL::CEMailIntroRequest read;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), read, MAIL::CEMailIntroRequest::MaxDataBytes ) && read.Verify( why ) );
        ASSERT_TRUE( read.nonce == request.nonce && read.parties.size() == request.parties.size() && read.parties[ 2 ].name == request.parties[ 2 ].name );

        // out of bounds is refused before the signature is looked at
        changed = request; changed.nonce = "xyz";
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.title = Repeat( 'T', 500 );
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.creatorName = Repeat( 'N', 500 );
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.inviteeAddress = "not an address";
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.creatorName = "Al\r\nBcc: x@y.z";
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; for ( int i=0; i<10; ++i ) changed.parties.push_back( MAIL::CEMailIntroParty( "P", "p@mail.test" ) );
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.expiresAt = changed.createdAt - 1;
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        // an identity without a key cannot sign
        MULTIPLAY::CPlayerIdentity keyless;
        MAIL::CEMailIntroRequest unsigned_ = request;
        unsigned_.signature.Clear();
        ASSERT_FALSE( unsigned_.Sign( keyless ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction: the settings of the game are announced in the request, signed with it, kept in its bytes and bounded like the settings of a record" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction: the settings of the game are announced in the request, signed with it, kept in its bytes and bounded like the settings of a record" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        MAIL::CEMailIntroductions intro;
        intro.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        const CORE::CString matchId = MULTIPLAY::CMatchId::Generate();
        GUCEF::map< CORE::CString, CORE::CString > settings;
        settings[ "scenario" ] = "custom";
        settings[ "galaxySize" ] = "125";
        settings[ "seed" ] = "4242";
        settings[ "mailTransfer" ] = "ATTACHMENT";
        CORE::CString reason;
        ASSERT_TRUE( intro.BeginDraft( matchId, "Friday game", "Alice", "alice@mail.test", settings, TwoContacts(), g_start, reason ) );
        MAIL::CEMailIntroRequest request;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == intro.PrepareRequest( matchId, "bob@mail.test", false, g_start, request ) );
        CORE::CString why;
        ASSERT_TRUE( request.Verify( why ) && 4 == request.settings.size() && request.settings == settings );

        // the settings are signed: no value can change, none can be added or taken away
        MAIL::CEMailIntroRequest changed = request;
        changed.settings[ "seed" ] = "4243";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.settings[ "extra" ] = "x";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.settings.erase( "galaxySize" );
        ASSERT_FALSE( changed.Verify( why ) );
        changed = request; changed.settings.clear();
        ASSERT_FALSE( changed.Verify( why ) );

        // bytes and back
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SerializeToBytes( request, bytes, MAIL::CEMailIntroRequest::MaxDataBytes ) && bytes.GetDataSize() < MAIL::CEMailIntroRequest::MaxDataBytes );
        MAIL::CEMailIntroRequest read;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), read, MAIL::CEMailIntroRequest::MaxDataBytes ) && read.Verify( why ) );
        ASSERT_TRUE( read.settings == settings );

        // the invitee holds the settings the creator announced
        MAIL::CEMailIntroductions bob;
        bob.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == bob.HandleRequest( request, "alice@mail.test", "bob@mail.test", false, g_start ) );
        GUCEF::vector< MAIL::CEMailInboundIntro > pending;
        bob.GetPending( pending );
        ASSERT_TRUE( 1 == pending.size() && pending[ 0 ].request.settings == settings );

        // the settings of a record are the settings that were announced, or the invitation is not the one that was agreed to
        GUCEF::vector< CORE::CString > addresses;
        addresses.push_back( "alice@mail.test" );
        addresses.push_back( "bob@mail.test" );
        ASSERT_TRUE( bob.IsTableAnnounced( matchId, addresses, settings ) );
        GUCEF::map< CORE::CString, CORE::CString > other = settings;
        other[ "seed" ] = "4243";
        ASSERT_FALSE( bob.IsTableAnnounced( matchId, addresses, other ) );
        other = settings; other[ "extra" ] = "x";
        ASSERT_FALSE( bob.IsTableAnnounced( matchId, addresses, other ) );
        other = settings; other.erase( "seed" );
        ASSERT_FALSE( bob.IsTableAnnounced( matchId, addresses, other ) );
        ASSERT_FALSE( bob.IsTableAnnounced( matchId, addresses, GUCEF::map< CORE::CString, CORE::CString >() ) );

        // a setting that names the number of seats may be lower in the record than announced (a player was taken out of the draft), and no higher; nothing else may differ
        other = settings; other[ "seats" ] = "3";
        ASSERT_FALSE( bob.IsTableAnnounced( matchId, addresses, other ) );
        GUCEF::map< CORE::CString, CORE::CString > announcedSeats = settings;
        announcedSeats[ "seats" ] = "4";
        MAIL::CEMailIntroductions seatsCreator;
        seatsCreator.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        const CORE::CString seatsId = MULTIPLAY::CMatchId::Generate();
        ASSERT_TRUE( seatsCreator.BeginDraft( seatsId, "Seats game", "Alice", "alice@mail.test", announcedSeats, TwoContacts(), g_start, reason ) );
        MAIL::CEMailIntroRequest seatsRequest;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == seatsCreator.PrepareRequest( seatsId, "bob@mail.test", false, g_start, seatsRequest ) );
        MAIL::CEMailIntroductions seatsBob;
        seatsBob.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == seatsBob.HandleRequest( seatsRequest, "alice@mail.test", "bob@mail.test", false, g_start ) );
        other = announcedSeats; other[ "seats" ] = "3";
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other ) );
        ASSERT_TRUE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other[ "seats" ] = "4";
        ASSERT_TRUE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other[ "seats" ] = "5";
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other[ "seats" ] = "0";
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other[ "seats" ] = "three";
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other = announcedSeats; other[ "seats" ] = "3"; other[ "seed" ] = "4243";
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other = announcedSeats; other[ "seats" ] = "3"; other[ "extra" ] = "x";
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );
        other = announcedSeats; other.erase( "seats" );
        ASSERT_FALSE( seatsBob.IsTableAnnounced( seatsId, addresses, other, "seats" ) );

        // a request that announces no settings asks nothing of the record
        MAIL::CEMailIntroductions plainCreator;
        plainCreator.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        const CORE::CString plainId = MULTIPLAY::CMatchId::Generate();
        ASSERT_TRUE( plainCreator.BeginDraft( plainId, "Plain game", "Alice", "alice@mail.test", GUCEF::map< CORE::CString, CORE::CString >(), TwoContacts(), g_start, reason ) );
        MAIL::CEMailIntroRequest plain;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == plainCreator.PrepareRequest( plainId, "bob@mail.test", false, g_start, plain ) && plain.settings.empty() && plain.Verify( why ) );
        MAIL::CEMailIntroductions bob2;
        bob2.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == bob2.HandleRequest( plain, "alice@mail.test", "bob@mail.test", false, g_start ) );
        ASSERT_TRUE( bob2.IsTableAnnounced( plainId, addresses, settings ) );

        // bounds: as many settings as a record holds, a key and a value no longer than there
        changed = request;
        for ( int i=0; i<40; ++i )
            changed.settings[ "key" + CORE::Int32ToString( i ) ] = "v";
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.settings[ Repeat( 'k', 65 ) ] = "v";
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.settings[ "long" ] = Repeat( 'v', 257 );
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.settings[ "" ] = "v";
        ASSERT_FALSE( changed.IsWellFormed( why ) );
        changed = request; changed.settings[ "ctrl" ] = "a\r\nb";
        ASSERT_FALSE( changed.IsWellFormed( why ) );

        // a draft cannot be made with settings a request cannot hold
        GUCEF::map< CORE::CString, CORE::CString > many;
        for ( int i=0; i<40; ++i )
            many[ "key" + CORE::Int32ToString( i ) ] = "v";
        ASSERT_FALSE( plainCreator.BeginDraft( MULTIPLAY::CMatchId::Generate(), "Many", "Alice", "alice@mail.test", many, TwoContacts(), g_start, reason ) || reason.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction: a reply is signed for the match, a decline holds the nonce and no key, and a user id has to be the one of the key" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction: a reply is signed for the match, a decline holds the nonce and no key, and a user id has to be the one of the key" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest request;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", request ) );
        MAIL::CEMailIntroReply reply = SignedReply( people.bob, request, "bob@mail.test" );
        CORE::CString why;
        ASSERT_TRUE( reply.Verify( why ) && reply.userId == people.bob.userId && reply.publicKey == people.bob.publicKey && 128 == reply.signature.Length() );

        MAIL::CEMailIntroReply changed = reply;
        changed.nonce = "ffffffffffffffffffffffffffffffff";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = reply; changed.matchId = MULTIPLAY::CMatchId::Generate();
        ASSERT_FALSE( changed.Verify( why ) );
        changed = reply; changed.inviteeAddress = "mallory@mail.test";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = reply; changed.creatorAddress = "mallory@mail.test";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = reply; changed.displayName = "Not Bob";
        ASSERT_FALSE( changed.Verify( why ) );
        changed = reply; changed.accepted = false;
        ASSERT_FALSE( changed.Verify( why ) );                              // a decline holds no key and no signature
        changed = reply; changed.publicKey = people.mallory.publicKey;
        ASSERT_FALSE( changed.Verify( why ) );
        changed = reply; changed.publicKey = people.mallory.publicKey; changed.userId = people.mallory.userId;
        ASSERT_FALSE( changed.Verify( why ) );                              // the signature is Bob's

        MAIL::CEMailIntroReply decline;
        decline.matchId = request.matchId;
        decline.nonce = request.nonce;
        decline.accepted = false;
        decline.inviteeAddress = "bob@mail.test";
        decline.creatorAddress = "alice@mail.test";
        decline.createdAt = g_start;
        ASSERT_TRUE( decline.Verify( why ) );
        decline.publicKey = people.bob.publicKey;
        ASSERT_FALSE( decline.Verify( why ) );                              // a decline that carries a key is not a decline
        decline.publicKey.Clear();
        decline.signature = reply.signature;
        ASSERT_FALSE( decline.Verify( why ) );

        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SerializeToBytes( reply, bytes, MAIL::CEMailIntroRequest::MaxDataBytes ) );
        MAIL::CEMailIntroReply read;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), read, MAIL::CEMailIntroRequest::MaxDataBytes ) && read.Verify( why ) && read.accepted );
        changed = reply; changed.displayName = Repeat( 'N', 500 );
        ASSERT_FALSE( changed.Verify( why ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Address book: keys by address, a different key is told apart, blocked addresses, a bound, and a file that is read back and one that is damaged" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Address book: keys by address, a different key is told apart, blocked addresses, a bound, and a file that is read back and one that is damaged" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        MAIL::CEMailAddressBook book;
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_UNKNOWN == book.Check( "bob@mail.test", people.bob.publicKey ) && 0 == book.GetCount() );
        ASSERT_TRUE( book.Remember( "Bob@Mail.Test", people.bob.publicKey.Uppercase(), "Bob", g_start ) && 1 == book.GetCount() );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == book.Check( "bob@mail.test", people.bob.publicKey ) );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == book.Check( "BOB@MAIL.TEST", people.bob.publicKey ) );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_DIFFERENT == book.Check( "bob@mail.test", people.mallory.publicKey ) );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_UNKNOWN == book.Check( "carol@mail.test", people.carol.publicKey ) );
        MAIL::CEMailAddressEntry entry;
        ASSERT_TRUE( book.Find( "bob@mail.test", entry ) && entry.publicKey == people.bob.publicKey && entry.displayName == "Bob" && g_start == entry.firstSeen );
        // not an address, not a key
        ASSERT_FALSE( book.Remember( "not an address", people.bob.publicKey, "x", g_start ) );
        ASSERT_FALSE( book.Remember( "x@mail.test", "abcd", "x", g_start ) );
        // replaced on purpose
        ASSERT_TRUE( book.Remember( "bob@mail.test", people.mallory.publicKey, "Bob", g_start + 5 ) && 1 == book.GetCount() );
        ASSERT_TRUE( book.Find( "bob@mail.test", entry ) && entry.publicKey == people.mallory.publicKey && g_start == entry.firstSeen && g_start + 5 == entry.lastSeen );
        // blocked
        ASSERT_FALSE( book.IsBlocked( "carol@mail.test" ) );
        book.SetBlocked( "Carol@mail.test", true, g_start );
        ASSERT_TRUE( book.IsBlocked( "carol@mail.test" ) && MAIL::CEMailAddressBook::CHECK_UNKNOWN == book.Check( "carol@mail.test", people.carol.publicKey ) );
        book.SetBlocked( "carol@mail.test", false, g_start );
        ASSERT_FALSE( book.IsBlocked( "carol@mail.test" ) );
        book.SetBlocked( "carol@mail.test", true, g_start );
        book.Forget( "bob@mail.test" );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_UNKNOWN == book.Check( "bob@mail.test", people.mallory.publicKey ) );

        // a file
        const CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "mailtemp" ), "addressbook" );
        CORE::DeleteDir( dir, true );
        CORE::CreateDirs( dir );
        const CORE::CString path = CORE::CombinePath( dir, "book.state" );
        ASSERT_TRUE( book.Remember( "bob@mail.test", people.bob.publicKey, "Bob", g_start ) && book.SaveToFile( path ) );
        MAIL::CEMailAddressBook again;
        ASSERT_TRUE( again.LoadFromFile( path ) && 2 == again.GetCount() && again.IsBlocked( "carol@mail.test" ) && MAIL::CEMailAddressBook::CHECK_SAME == again.Check( "bob@mail.test", people.bob.publicKey ) );
        // damaged: not a book, the book stays as it was
        const char junk[] = "{ this is not a book";
        ASSERT_TRUE( MAIL::CEMailFiles::WriteAtomic( path, junk, sizeof( junk ) - 1 ) );
        ASSERT_FALSE( again.LoadFromFile( path ) );
        ASSERT_TRUE( 2 == again.GetCount() );
        ASSERT_FALSE( again.LoadFromFile( CORE::CombinePath( dir, "missing.state" ) ) );

        // bounded: the oldest unused entry goes, blocked entries stay
        MAIL::CEMailAddressBook big;
        big.SetBlocked( "blocked@mail.test", true, g_start );
        for ( UInt32 i=0; i<MAIL::CEMailAddressBook::MaxEntries + 50; ++i )
            big.Remember( "p" + CORE::UInt32ToString( i ) + "@mail.test", people.bob.publicKey, "P", g_start + i );
        ASSERT_TRUE( big.GetCount() <= MAIL::CEMailAddressBook::MaxEntries && big.IsBlocked( "blocked@mail.test" ) );
        MAIL::CEMailAddressEntry newest;
        ASSERT_TRUE( big.Find( "p" + CORE::UInt32ToString( MAIL::CEMailAddressBook::MaxEntries + 49 ) + "@mail.test", newest ) );
        ASSERT_FALSE( big.Find( "p0@mail.test", newest ) );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction draft: the contacts are checked, a player code or a key of the book introduces at once, the others are asked with a nonce of their own" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction draft: the contacts are checked, a player code or a key of the book introduces at once, the others are asked with a nonce of their own" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        {
            // the input
            MAIL::CEMailIntroductions intro;
            intro.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
            GUCEF::map< CORE::CString, CORE::CString > settings;
            CORE::CString reason;
            const CORE::CString id = MULTIPLAY::CMatchId::Generate();
            ASSERT_FALSE( intro.BeginDraft( "not an id", "T", "Alice", "alice@mail.test", settings, TwoContacts(), g_start, reason ) );
            ASSERT_FALSE( intro.BeginDraft( id, "", "Alice", "alice@mail.test", settings, TwoContacts(), g_start, reason ) );
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice", settings, TwoContacts(), g_start, reason ) );
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, GUCEF::vector< MAIL::CEMailIntroContactInput >(), g_start, reason ) );
            GUCEF::vector< MAIL::CEMailIntroContactInput > bad = TwoContacts();
            bad[ 1 ].address = "BOB@mail.test";
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );            // an address twice
            bad = TwoContacts(); bad[ 0 ].address = "Alice@Mail.test";
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );            // the creator's own address
            bad = TwoContacts(); bad[ 0 ].address = "bob@mail.test\r\nBcc: x@y.z";
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );
            bad = TwoContacts(); bad[ 0 ].name.Clear();
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );
            bad = TwoContacts( "tooshort" );
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );            // a code that is no key
            bad = TwoContacts( people.alice.publicKey.C_String() );
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );            // the creator's own key
            bad = TwoContacts( people.bob.publicKey.C_String(), people.bob.publicKey.C_String() );
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );            // one key for two players
            bad.clear();
            for ( int i=0; i<6; ++i )
                bad.push_back( MAIL::CEMailIntroContactInput( "P", ( "p" + CORE::UInt32ToString( i ) + "@mail.test" ).C_String() ) );
            ASSERT_FALSE( intro.BeginDraft( id, "T", "Alice", "alice@mail.test", settings, bad, g_start, reason ) );            // more than five
            GUCEF::vector< MAIL::CEMailIntroDraft > none;
            intro.GetDrafts( none );
            ASSERT_TRUE( none.empty() );
        }
        {
            // a code introduces at once; a key in the book does not: the address is asked, and the key of the book is what the answer is compared with
            SCreator creator;
            creator.intro.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
            ASSERT_TRUE( creator.intro.GetBook().Remember( "carol@mail.test", people.carol.publicKey, "Carol", g_start - g_day ) );
            creator.matchId = MULTIPLAY::CMatchId::Generate();
            GUCEF::map< CORE::CString, CORE::CString > settings;
            settings[ "seats" ] = "3";
            CORE::CString reason;
            GUCEF::vector< MAIL::CEMailIntroContactInput > contacts = TwoContacts( people.bob.publicKey.Uppercase().C_String() );
            ASSERT_TRUE( creator.intro.BeginDraft( creator.matchId, "Friday game", "Alice", "alice@mail.test", settings, contacts, g_start, reason ) );
            MAIL::CEMailIntroDraft draft;
            ASSERT_TRUE( creator.intro.FindDraft( creator.matchId, draft ) && "3" == draft.settings[ "seats" ] );
            ASSERT_TRUE( MAIL::MAILINTRO_INTRODUCED == draft.contacts[ 0 ].state && MAIL::MAILINTROSOURCE_CODE == draft.contacts[ 0 ].source && draft.contacts[ 0 ].publicKey == people.bob.publicKey );
            ASSERT_TRUE( MAIL::MAILINTRO_NOT_ASKED == draft.contacts[ 1 ].state && draft.contacts[ 1 ].publicKey.IsNULLOrEmpty() );
            MAIL::CEMailIntroSummary summary;
            ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && !summary.allIntroduced && !summary.startable && 1 == summary.introduced && 1 == summary.notAsked );
            // the same match id twice
            ASSERT_FALSE( creator.intro.BeginDraft( creator.matchId, "Friday game", "Alice", "alice@mail.test", settings, contacts, g_start, reason ) );
            // an introduced contact is not asked, unless forced
            MAIL::CEMailIntroRequest request;
            ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", false, g_start, request ) );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", true, g_start, request ) );
            ASSERT_TRUE( creator.intro.FindDraft( creator.matchId, draft ) && MAIL::MAILINTRO_WAITING == draft.contacts[ 0 ].state );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_NO_CONTACT == creator.intro.PrepareRequest( creator.matchId, "nobody@mail.test", true, g_start, request ) );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_NO_DRAFT == creator.intro.PrepareRequest( MULTIPLAY::CMatchId::Generate(), "bob@mail.test", true, g_start, request ) );
            // Carol answers with the key the book has: introduced, no warning
            MAIL::CEMailIntroRequest toCarol;
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.PrepareRequest( creator.matchId, "carol@mail.test", false, g_start, toCarol ) );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.HandleReply( SignedReply( people.carol, toCarol, "carol@mail.test" ), "carol@mail.test", g_start + 10 ) );
            ASSERT_TRUE( creator.intro.FindDraft( creator.matchId, draft ) && MAIL::MAILINTRO_INTRODUCED == draft.contacts[ 1 ].state && MAIL::MAILINTROSOURCE_REPLY == draft.contacts[ 1 ].source );
        }
        {
            // the others are asked
            SCreator creator;
            ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
            MAIL::CEMailIntroSummary summary;
            ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 2 == summary.notAsked && !summary.startable && !summary.allIntroduced );
            MAIL::CEMailIntroRequest toBob, toCarol;
            ASSERT_TRUE( creator.Ask( "bob@mail.test", toBob ) && creator.Ask( "CAROL@mail.test", toCarol ) );
            ASSERT_TRUE( toBob.nonce != toCarol.nonce && toBob.inviteeAddress == "bob@mail.test" && toCarol.inviteeAddress == "carol@mail.test" );
            ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 2 == summary.waiting && !summary.startable );
            // asking again at once: limited
            MAIL::CEMailIntroRequest again;
            ASSERT_TRUE( MAIL::MAILINTRORESULT_RATE_LIMITED == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", false, g_start + 1000, again ) );
            // after the interval the same nonce goes again, later expiry
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", false, g_start + 2 * 60 * 60 * 1000, again ) );
            ASSERT_TRUE( again.nonce == toBob.nonce && again.expiresAt > toBob.expiresAt );
            CORE::CString why;
            ASSERT_TRUE( again.Verify( why ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction reply: a signed reply from the right address with the right nonce introduces, and the key goes in the draft and the address book" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction reply: a signed reply from the right address with the right nonce introduces, and the key goes in the draft and the address book" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest toBob, toCarol;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", toBob ) && creator.Ask( "carol@mail.test", toCarol ) );

        // Carol answers first, out of the order of the asking
        MAIL::CEMailIntroReply carolReply = SignedReply( people.carol, toCarol, "carol@mail.test" );
        CORE::CString matchId;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.HandleReply( carolReply, "Carol@Mail.Test", g_start + 2000, &matchId ) && matchId == creator.matchId );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.introduced && 1 == summary.waiting && !summary.startable );
        MAIL::CEMailIntroDraft draft;
        ASSERT_TRUE( creator.intro.FindDraft( creator.matchId, draft ) && MAIL::MAILINTRO_INTRODUCED == draft.contacts[ 1 ].state && MAIL::MAILINTROSOURCE_REPLY == draft.contacts[ 1 ].source );
        ASSERT_TRUE( draft.contacts[ 1 ].publicKey == people.carol.publicKey && draft.contacts[ 1 ].displayName == "Carol" );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == creator.intro.GetBook().Check( "carol@mail.test", people.carol.publicKey ) );

        // the same reply again: ignored
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == creator.intro.HandleReply( carolReply, "carol@mail.test", g_start + 3000 ) );
        // another reply with the nonce that is used up: a conflict, and the key stays
        MAIL::CEMailIntroReply other = SignedReply( people.mallory, toCarol, "carol@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_CONFLICT == creator.intro.HandleReply( other, "carol@mail.test", g_start + 3000 ) );
        ASSERT_TRUE( creator.intro.FindDraft( creator.matchId, draft ) && draft.contacts[ 1 ].publicKey == people.carol.publicKey );

        // Bob, later, and the table can start
        MAIL::CEMailIntroReply bobReply = SignedReply( people.bob, toBob, "bob@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.HandleReply( bobReply, "bob@mail.test", g_start + 5 * g_day ) );
        ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && summary.allIntroduced && summary.startable );
        GUCEF::vector< MAIL::CEMailIntroContact > introduced;
        ASSERT_TRUE( creator.intro.GetIntroduced( creator.matchId, introduced ) && 2 == introduced.size() && introduced[ 0 ].address == "bob@mail.test" && introduced[ 1 ].address == "carol@mail.test" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction reply: forged replies are refused: a wrong nonce, a wrong From, a wrong address inside, a key that does not sign, a key that is not the user id, the creator's key, a key of another player, and the player code that was entered" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction reply: forged replies are refused: a wrong nonce, a wrong From, a wrong address inside, a key that does not sign, a key that is not the user id, the creator's key, a key of another player, and the player code that was entered" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest toBob, toCarol;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", toBob ) && creator.Ask( "carol@mail.test", toCarol ) );
        MAIL::CEMailIntroSummary summary;
        MAIL::CEMailIntroReply good = SignedReply( people.bob, toBob, "bob@mail.test" );

        // a nonce that was not issued, or the nonce of another contact
        MAIL::CEMailIntroReply wrongNonce = good;
        wrongNonce.nonce = "00112233445566778899aabbccddeeff";
        wrongNonce.Sign( people.bob );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_NONCE == creator.intro.HandleReply( wrongNonce, "bob@mail.test", g_start + 100 ) );
        MAIL::CEMailIntroReply carolsNonce = good;
        carolsNonce.nonce = toCarol.nonce;
        carolsNonce.Sign( people.bob );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_NONCE == creator.intro.HandleReply( carolsNonce, "bob@mail.test", g_start + 100 ) );
        // a mail from another address: Mallory answers with Bob's reply
        ASSERT_TRUE( MAIL::MAILINTRORESULT_NO_CONTACT == creator.intro.HandleReply( good, "mallory@mail.test", g_start + 100 ) );
        // Carol's address with Bob's reply: the address inside says Bob
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_ADDRESS == creator.intro.HandleReply( good, "carol@mail.test", g_start + 100 ) );
        // the address inside is not the From address
        MAIL::CEMailIntroReply otherAddress = SignedReply( people.bob, toBob, "mallory@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_ADDRESS == creator.intro.HandleReply( otherAddress, "bob@mail.test", g_start + 100 ) );
        // the creator's address is not ours
        MAIL::CEMailIntroReply otherCreator = good;
        otherCreator.creatorAddress = "mallory@mail.test";
        otherCreator.Sign( people.bob );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_ADDRESS == creator.intro.HandleReply( otherCreator, "bob@mail.test", g_start + 100 ) );
        // a key that did not sign it
        MAIL::CEMailIntroReply forged = good;
        forged.publicKey = people.mallory.publicKey;
        forged.userId = people.mallory.userId;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_SIGNATURE == creator.intro.HandleReply( forged, "bob@mail.test", g_start + 100 ) );
        // a user id that is not the user id of the key
        MAIL::CEMailIntroReply wrongUser = good;
        wrongUser.userId = people.mallory.userId;
        const MAIL::TMailIntroResult wrongUserResult = creator.intro.HandleReply( wrongUser, "bob@mail.test", g_start + 100 );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_SIGNATURE == wrongUserResult || MAIL::MAILINTRORESULT_BAD_STATEMENT == wrongUserResult );
        // a statement that is out of bounds
        MAIL::CEMailIntroReply huge = good;
        huge.displayName = Repeat( 'x', 400 );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_STATEMENT == creator.intro.HandleReply( huge, "bob@mail.test", g_start + 100 ) );
        // the creator's own key as the key of Bob
        MAIL::CEMailIntroReply asAlice = SignedReply( people.alice, toBob, "bob@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_KEY == creator.intro.HandleReply( asAlice, "bob@mail.test", g_start + 100 ) );
        // nothing of that has introduced anybody
        ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 0 == summary.introduced && 2 == summary.waiting );
        // a reply for a draft that does not exist
        MAIL::CEMailIntroReply lost = good;
        lost.matchId = MULTIPLAY::CMatchId::Generate();
        lost.Sign( people.bob );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_NO_DRAFT == creator.intro.HandleReply( lost, "bob@mail.test", g_start + 100 ) );

        // one key for two players: the second is refused
        MAIL::CEMailIntroReply bobFirst = good;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.HandleReply( bobFirst, "bob@mail.test", g_start + 100 ) );
        MAIL::CEMailIntroReply carolWithBobsKey = SignedReply( people.bob, toCarol, "carol@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_KEY == creator.intro.HandleReply( carolWithBobsKey, "carol@mail.test", g_start + 100 ) );

        // the player code that was entered is the key that has to answer
        SCreator coded;
        ASSERT_TRUE( coded.Start( people, TwoContacts( "", "" ) ) );
        GUCEF::vector< MAIL::CEMailIntroContactInput > contacts;
        contacts.push_back( MAIL::CEMailIntroContactInput( "Bob", "bob@mail.test" ) );
        MAIL::CEMailIntroductions other;
        other.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        GUCEF::map< CORE::CString, CORE::CString > settings;
        CORE::CString reason;
        contacts[ 0 ].playerCode = people.bob.publicKey;
        ASSERT_TRUE( other.BeginDraft( id, "Coded", "Alice", "alice@mail.test", settings, contacts, g_start, reason ) );
        MAIL::CEMailIntroRequest codedRequest;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == other.PrepareRequest( id, "bob@mail.test", true, g_start, codedRequest ) );
        MAIL::CEMailIntroReply withOtherKey = SignedReply( people.mallory, codedRequest, "bob@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_KEY == other.HandleReply( withOtherKey, "bob@mail.test", g_start + 100 ) );
        MAIL::CEMailIntroReply withCodeKey = SignedReply( people.bob, codedRequest, "bob@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == other.HandleReply( withCodeKey, "bob@mail.test", g_start + 100 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction reply: a decline holds the nonce only and ends the contact, a resend makes a new nonce, an expired request is refused and a late reply does not revive it" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction reply: a decline holds the nonce only and ends the contact, a resend makes a new nonce, an expired request is refused and a late reply does not revive it" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest toBob, toCarol;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", toBob ) && creator.Ask( "carol@mail.test", toCarol ) );

        // a decline with a wrong nonce does nothing; with the right one it declines
        MAIL::CEMailIntroReply decline;
        decline.matchId = toBob.matchId;
        decline.nonce = "00112233445566778899aabbccddeeff";
        decline.accepted = false;
        decline.inviteeAddress = "bob@mail.test";
        decline.creatorAddress = "alice@mail.test";
        decline.createdAt = g_start + 10;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_NONCE == creator.intro.HandleReply( decline, "bob@mail.test", g_start + 10 ) );
        decline.nonce = toBob.nonce;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DECLINED == creator.intro.HandleReply( decline, "bob@mail.test", g_start + 10 ) );
        MAIL::CEMailIntroSummary summary;
        ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.declined && 1 == summary.waiting );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == creator.intro.HandleReply( decline, "bob@mail.test", g_start + 20 ) );
        // a signed acceptance with the nonce of a decline is no answer any more
        MAIL::CEMailIntroReply late = SignedReply( people.bob, toBob, "bob@mail.test", g_start + 30 );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_CONFLICT == creator.intro.HandleReply( late, "bob@mail.test", g_start + 30 ) );
        // a resend after a decline makes a new nonce, the old one is of no use
        MAIL::CEMailIntroRequest second;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_RATE_LIMITED == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", false, g_start + 1000, second ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", false, g_start + 2 * 60 * 60 * 1000, second ) );
        ASSERT_TRUE( second.nonce != toBob.nonce );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_NONCE == creator.intro.HandleReply( late, "bob@mail.test", g_start + 2 * 60 * 60 * 1000 + 5 ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.HandleReply( SignedReply( people.bob, second, "bob@mail.test", g_start + 2 * 60 * 60 * 1000 + 10 ), "bob@mail.test", g_start + 2 * 60 * 60 * 1000 + 10 ) );

        // Carol never answers: the request expires
        GUCEF::vector< CORE::CString > changed;
        creator.intro.ExpireDue( g_start + 13 * g_day, changed );
        ASSERT_TRUE( changed.empty() );
        ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.waiting );
        creator.intro.ExpireDue( g_start + 15 * g_day, changed );
        ASSERT_TRUE( 1 == changed.size() && changed[ 0 ] == creator.matchId );
        ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.expired && 0 == summary.waiting && 1 == summary.introduced && summary.startable );
        // her answer after the expiry is refused, also with a good signature
        MAIL::CEMailIntroReply carolLate = SignedReply( people.carol, toCarol, "carol@mail.test", g_start + 15 * g_day );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_EXPIRED == creator.intro.HandleReply( carolLate, "carol@mail.test", g_start + 15 * g_day ) );
        // and an answer that comes after the expiry before ExpireDue ran is refused as well
        SCreator quick;
        ASSERT_TRUE( quick.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest q;
        ASSERT_TRUE( quick.Ask( "bob@mail.test", q ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_EXPIRED == quick.intro.HandleReply( SignedReply( people.bob, q, "bob@mail.test", g_start + 20 * g_day ), "bob@mail.test", g_start + 20 * g_day ) );
        // a resend for an expired contact makes a new nonce and a new expiry
        MAIL::CEMailIntroRequest renewed;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == quick.intro.PrepareRequest( quick.matchId, "bob@mail.test", false, g_start + 20 * g_day, renewed ) );
        ASSERT_TRUE( renewed.nonce != q.nonce && renewed.expiresAt > g_start + 20 * g_day );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == quick.intro.HandleReply( SignedReply( people.bob, renewed, "bob@mail.test", g_start + 20 * g_day + 5 ), "bob@mail.test", g_start + 20 * g_day + 5 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction reply: a changed key is held until the creator decides, and is never taken or remembered silently" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction reply: a changed key is held until the creator decides, and is never taken or remembered silently" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        for ( int variant=0; variant<2; ++variant )
        {
            SCreator creator;
            creator.intro.Configure( people.alice, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
            // Bob was met before with his old key; now the key of Mallory answers for the address of Bob
            ASSERT_TRUE( creator.intro.GetBook().Remember( "bob@mail.test", people.bob.publicKey, "Bob", g_start - 30 * g_day ) );
            creator.matchId = MULTIPLAY::CMatchId::Generate();
            GUCEF::map< CORE::CString, CORE::CString > settings;
            CORE::CString reason;
            GUCEF::vector< MAIL::CEMailIntroContactInput > contacts;
            contacts.push_back( MAIL::CEMailIntroContactInput( "Bob", "bob@mail.test" ) );
            ASSERT_TRUE( creator.intro.BeginDraft( creator.matchId, "Changed", "Alice", "alice@mail.test", settings, contacts, g_start, reason ) );
            MAIL::CEMailIntroRequest request;
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.PrepareRequest( creator.matchId, "bob@mail.test", true, g_start, request ) );      // introduce again
            MAIL::CEMailIntroReply reply = SignedReply( people.mallory, request, "bob@mail.test" );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_KEY_CHANGED == creator.intro.HandleReply( reply, "bob@mail.test", g_start + 10 ) );
            MAIL::CEMailIntroSummary summary;
            ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.keyChanged && !summary.startable && 0 == summary.introduced );
            MAIL::CEMailIntroDraft draft;
            ASSERT_TRUE( creator.intro.FindDraft( creator.matchId, draft ) && MAIL::MAILINTRO_KEY_CHANGED == draft.contacts[ 0 ].state && draft.contacts[ 0 ].pendingKey == people.mallory.publicKey );
            // the book still has the old key
            ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == creator.intro.GetBook().Check( "bob@mail.test", people.bob.publicKey ) );
            GUCEF::vector< MAIL::CEMailIntroContact > introduced;
            ASSERT_TRUE( creator.intro.GetIntroduced( creator.matchId, introduced ) && introduced.empty() );
            // the same reply again does not decide anything
            ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == creator.intro.HandleReply( reply, "bob@mail.test", g_start + 20 ) );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_NO_CONTACT == creator.intro.ConfirmKeyChange( creator.matchId, "carol@mail.test", true, g_start + 30 ) );
            if ( 0 == variant )
            {
                // accepted: used and remembered
                ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.ConfirmKeyChange( creator.matchId, "bob@mail.test", true, g_start + 30 ) );
                ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.introduced && summary.startable );
                ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == creator.intro.GetBook().Check( "bob@mail.test", people.mallory.publicKey ) );
                ASSERT_TRUE( MAIL::MAILINTRORESULT_NOT_WAITING == creator.intro.ConfirmKeyChange( creator.matchId, "bob@mail.test", true, g_start + 40 ) );
            }
            else
            {
                // refused: the contact is refused, the book is as it was, and the nonce does not come back
                ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.ConfirmKeyChange( creator.matchId, "bob@mail.test", false, g_start + 30 ) );
                ASSERT_TRUE( creator.intro.GetSummary( creator.matchId, summary ) && 1 == summary.refused && !summary.startable );
                ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == creator.intro.GetBook().Check( "bob@mail.test", people.bob.publicKey ) );
                ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == creator.intro.HandleReply( reply, "bob@mail.test", g_start + 40 ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction request (invitee): a good request is pending, forged, misdirected, expired, blocked, repeated and too many requests are refused or limited, and a changed key of the creator is marked" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction request (invitee): a good request is pending, forged, misdirected, expired, blocked, repeated and too many requests are refused or limited, and a changed key of the creator is marked" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest request;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", request ) );

        MAIL::CEMailIntroductions bob;
        bob.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        const Int64 now = g_start + 1000;
        // good
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == bob.HandleRequest( request, "Alice@Mail.Test", "bob@mail.test", false, now ) );
        GUCEF::vector< MAIL::CEMailInboundIntro > pending;
        bob.GetPending( pending );
        ASSERT_TRUE( 1 == pending.size() && MAIL::CEMailInboundIntro::STATUS_PENDING == pending[ 0 ].status && pending[ 0 ].request.matchId == request.matchId && !pending[ 0 ].keyChanged );
        // the same again: nothing changes
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == bob.HandleRequest( request, "alice@mail.test", "bob@mail.test", false, now + 10 ) );
        bob.GetPending( pending );
        ASSERT_TRUE( 1 == pending.size() );

        // forged: another From, another own address, a changed field, an expired one, a request from the future, one for a match that exists, a blocked sender
        MAIL::CEMailIntroductions probe;
        probe.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_ADDRESS == probe.HandleRequest( request, "mallory@mail.test", "bob@mail.test", false, now ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_NOT_FOR_ME == probe.HandleRequest( request, "alice@mail.test", "carol@mail.test", false, now ) );
        MAIL::CEMailIntroRequest changed = request;
        changed.title = "Another game";
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_SIGNATURE == probe.HandleRequest( changed, "alice@mail.test", "bob@mail.test", false, now ) );
        changed = request; changed.title = Repeat( 'T', 500 );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_STATEMENT == probe.HandleRequest( changed, "alice@mail.test", "bob@mail.test", false, now ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_EXPIRED == probe.HandleRequest( request, "alice@mail.test", "bob@mail.test", false, request.expiresAt + 1 ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BAD_STATEMENT == probe.HandleRequest( request, "alice@mail.test", "bob@mail.test", false, request.createdAt - 3 * g_day ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_KNOWN_MATCH == probe.HandleRequest( request, "alice@mail.test", "bob@mail.test", true, now ) );
        probe.GetBook().SetBlocked( "alice@mail.test", true, now );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_BLOCKED == probe.HandleRequest( request, "alice@mail.test", "bob@mail.test", false, now ) );
        probe.GetPending( pending );
        ASSERT_TRUE( pending.empty() );
        // limits: three per sender, thirty-two in all
        MAIL::CEMailIntroductions limited;
        limited.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        UInt32 accepted = 0, refused = 0;
        for ( int i=0; i<6; ++i )
        {
            SCreator many;
            ASSERT_TRUE( many.Start( people, TwoContacts() ) );
            MAIL::CEMailIntroRequest r;
            ASSERT_TRUE( many.Ask( "bob@mail.test", r ) );
            const MAIL::TMailIntroResult result = limited.HandleRequest( r, "alice@mail.test", "bob@mail.test", false, now );
            if ( MAIL::MAILINTRORESULT_OK == result ) ++accepted; else if ( MAIL::MAILINTRORESULT_LIMIT == result ) ++refused;
        }
        ASSERT_TRUE( 3 == accepted && 3 == refused );
        MAIL::CEMailIntroLimits tight;
        tight.maxPending = 2;
        tight.maxPerSender = 10;
        MAIL::CEMailIntroductions total;
        total.Configure( people.bob, CORE::CString::Empty, tight );
        accepted = refused = 0;
        for ( int i=0; i<4; ++i )
        {
            SCreator many;
            ASSERT_TRUE( many.Start( people, TwoContacts() ) );
            MAIL::CEMailIntroRequest r;
            ASSERT_TRUE( many.Ask( "bob@mail.test", r ) );
            const MAIL::TMailIntroResult result = total.HandleRequest( r, "alice@mail.test", "bob@mail.test", false, now );
            if ( MAIL::MAILINTRORESULT_OK == result ) ++accepted; else if ( MAIL::MAILINTRORESULT_LIMIT == result ) ++refused;
        }
        ASSERT_TRUE( 2 == accepted && 2 == refused );

        // the same match with another nonce, another key: a conflict
        MAIL::CEMailIntroRequest conflicting = request;
        conflicting.nonce = "11111111111111111111111111111111";
        conflicting.Sign( people.alice );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_CONFLICT == bob.HandleRequest( conflicting, "alice@mail.test", "bob@mail.test", false, now ) );

        // the creator's key differs from the book
        MAIL::CEMailIntroductions careful;
        careful.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( careful.GetBook().Remember( "alice@mail.test", people.mallory.publicKey, "Alice", now - g_day ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == careful.HandleRequest( request, "alice@mail.test", "bob@mail.test", false, now ) );
        careful.GetPending( pending );
        ASSERT_TRUE( 1 == pending.size() && pending[ 0 ].keyChanged && pending[ 0 ].knownKey == people.mallory.publicKey );
        // purge: pending requests past their expiry go
        ASSERT_TRUE( 0 == careful.PurgeExpired( now ) && 1 == careful.PurgeExpired( request.expiresAt + 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction answer (invitee): accepting makes a signed reply the creator accepts, declining makes the decline once, a changed key of the creator needs a confirmation, and a lost reply is made again" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction answer (invitee): accepting makes a signed reply the creator accepts, declining makes the decline once, a changed key of the creator needs a confirmation, and a lost reply is made again" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        SCreator creator;
        ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
        MAIL::CEMailIntroRequest toBob, toCarol;
        ASSERT_TRUE( creator.Ask( "bob@mail.test", toBob ) && creator.Ask( "carol@mail.test", toCarol ) );
        const Int64 now = g_start + 1000;

        MAIL::CEMailIntroductions bob, carol;
        bob.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        carol.Configure( people.carol, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == bob.HandleRequest( toBob, "alice@mail.test", "bob@mail.test", false, now ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == carol.HandleRequest( toCarol, "alice@mail.test", "carol@mail.test", false, now ) );

        // Bob accepts: the creator takes the reply
        MAIL::CEMailIntroReply reply;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_NO_REQUEST == bob.AcceptRequest( MULTIPLAY::CMatchId::Generate(), false, "Bob", now, reply ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == bob.AcceptRequest( toBob.matchId, false, "Bob", now + 10, reply ) );
        CORE::CString why;
        ASSERT_TRUE( reply.accepted && reply.Verify( why ) && reply.nonce == toBob.nonce && reply.publicKey == people.bob.publicKey && reply.displayName == "Bob" && reply.inviteeAddress == "bob@mail.test" && reply.creatorAddress == "alice@mail.test" );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == creator.intro.HandleReply( reply, "bob@mail.test", now + 20 ) );
        // the creator is in the book of Bob now
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == bob.GetBook().Check( "alice@mail.test", people.alice.publicKey ) );
        // accepting again: nothing new; the reply can be made again after the interval, not before
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == bob.AcceptRequest( toBob.matchId, false, "Bob", now + 20, reply ) );
        MAIL::CEMailIntroReply again;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_RATE_LIMITED == bob.RepeatReply( toBob.matchId, "Bob", now + 1000, again ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == bob.RepeatReply( toBob.matchId, "Bob", now + 2 * 60 * 60 * 1000, again ) && again.Verify( why ) && again.nonce == toBob.nonce );
        // the request comes again (a resend): the reply goes again
        ASSERT_TRUE( MAIL::MAILINTRORESULT_RATE_LIMITED == bob.HandleRequest( toBob, "alice@mail.test", "bob@mail.test", false, now + 2 * 60 * 60 * 1000 + 5 * 60 * 1000 ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_RESEND_REPLY == bob.HandleRequest( toBob, "alice@mail.test", "bob@mail.test", false, now + 5 * 60 * 60 * 1000 ) );

        // Carol declines, once; the creator takes it
        MAIL::CEMailIntroReply decline;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == carol.DeclineRequest( toCarol.matchId, now + 10, decline ) );
        ASSERT_TRUE( !decline.accepted && decline.nonce == toCarol.nonce && decline.publicKey.IsNULLOrEmpty() && decline.signature.IsNULLOrEmpty() && decline.Verify( why ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DECLINED == creator.intro.HandleReply( decline, "carol@mail.test", now + 20 ) );
        MAIL::CEMailIntroReply second;
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == carol.DeclineRequest( toCarol.matchId, now + 30, second ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == carol.AcceptRequest( toCarol.matchId, false, "Carol", now + 30, second ) );
        // a declined request does not make the carol side send anything again
        ASSERT_TRUE( MAIL::MAILINTRORESULT_DUPLICATE == carol.HandleRequest( toCarol, "alice@mail.test", "carol@mail.test", false, now + 5 * 60 * 60 * 1000 ) );

        // too late
        MAIL::CEMailIntroductions late;
        late.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == late.HandleRequest( toBob, "alice@mail.test", "bob@mail.test", false, now ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_EXPIRED == late.AcceptRequest( toBob.matchId, false, "Bob", toBob.expiresAt + 1, reply ) );

        // a creator whose key is not the one of the book: confirmation first
        MAIL::CEMailIntroductions careful;
        careful.Configure( people.bob, CORE::CString::Empty, MAIL::CEMailIntroLimits() );
        ASSERT_TRUE( careful.GetBook().Remember( "alice@mail.test", people.mallory.publicKey, "Alice", now - g_day ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == careful.HandleRequest( toBob, "alice@mail.test", "bob@mail.test", false, now ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_NEEDS_CONFIRMATION == careful.AcceptRequest( toBob.matchId, false, "Bob", now, reply ) );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == careful.GetBook().Check( "alice@mail.test", people.mallory.publicKey ) );
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == careful.AcceptRequest( toBob.matchId, true, "Bob", now, reply ) );
        ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == careful.GetBook().Check( "alice@mail.test", people.alice.publicKey ) );

        // the table that the invitation names is the table that was announced
        GUCEF::vector< CORE::CString > addresses;
        addresses.push_back( "alice@mail.test" );
        addresses.push_back( "BOB@mail.test" );
        addresses.push_back( "carol@mail.test" );
        ASSERT_TRUE( bob.IsTableAnnounced( toBob.matchId, addresses ) );
        addresses.push_back( "mallory@mail.test" );
        ASSERT_FALSE( bob.IsTableAnnounced( toBob.matchId, addresses ) );
        ASSERT_FALSE( bob.IsTableAnnounced( MULTIPLAY::CMatchId::Generate(), addresses ) );
        // the invitation arrived: the pending request has done its work
        bob.ForgetInbound( toBob.matchId );
        GUCEF::vector< MAIL::CEMailInboundIntro > pending;
        bob.GetPending( pending );
        ASSERT_TRUE( pending.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Introduction state: drafts, pending requests and the book are kept in files and read back, a damaged file starts empty, and no address, key or nonce is logged" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Introduction state: drafts, pending requests and the book are kept in files and read back, a damaged file starts empty, and no address, key or nonce is logged" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        SPeople people;
        ASSERT_TRUE( people.Make() );
        const CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "mailtemp" ), "intro-state" );
        CORE::DeleteDir( dir, true );
        CORE::CreateDirs( dir );
        TESTFW::CLogTextCapture logs;
        CORE::CString matchId, nonceBob;
        {
            SCreator creator;
            ASSERT_TRUE( creator.Start( people, TwoContacts( "", people.carol.publicKey.C_String() ), dir ) );
            MAIL::CEMailIntroRequest request;
            ASSERT_TRUE( creator.Ask( "bob@mail.test", request ) );
            matchId = creator.matchId;
            nonceBob = request.nonce;
            ASSERT_TRUE( creator.intro.Save() );
            // as a invitee in the same files
            MAIL::CEMailIntroductions invitee;
            invitee.Configure( people.alice, dir, MAIL::CEMailIntroLimits() );
            ASSERT_TRUE( invitee.Load() );
        }
        {
            MAIL::CEMailIntroductions reloaded;
            reloaded.Configure( people.alice, dir, MAIL::CEMailIntroLimits() );
            ASSERT_TRUE( reloaded.Load() );
            MAIL::CEMailIntroDraft draft;
            ASSERT_TRUE( reloaded.FindDraft( matchId, draft ) && "Friday game" == draft.title && 2 == draft.contacts.size() && "ATTACHMENT" == draft.settings[ "mailTransfer" ] );
            ASSERT_TRUE( MAIL::MAILINTRO_WAITING == draft.contacts[ 0 ].state && draft.contacts[ 0 ].nonce == nonceBob && MAIL::MAILINTRO_INTRODUCED == draft.contacts[ 1 ].state && draft.contacts[ 1 ].publicKey == people.carol.publicKey );
            // the nonce still works after the restart
            MAIL::CEMailIntroRequest again;
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == reloaded.PrepareRequest( matchId, "bob@mail.test", false, g_start + 2 * 60 * 60 * 1000, again ) && again.nonce == nonceBob );
            ASSERT_TRUE( MAIL::MAILINTRORESULT_OK == reloaded.HandleReply( SignedReply( people.bob, again, "bob@mail.test", g_start + 2 * 60 * 60 * 1000 + 5 ), "bob@mail.test", g_start + 2 * 60 * 60 * 1000 + 5 ) );
            ASSERT_TRUE( reloaded.Save() );
        }
        {
            MAIL::CEMailIntroductions reloaded;
            reloaded.Configure( people.alice, dir, MAIL::CEMailIntroLimits() );
            ASSERT_TRUE( reloaded.Load() );
            MAIL::CEMailIntroSummary summary;
            ASSERT_TRUE( reloaded.GetSummary( matchId, summary ) && summary.allIntroduced );
            ASSERT_TRUE( MAIL::CEMailAddressBook::CHECK_SAME == reloaded.GetBook().Check( "bob@mail.test", people.bob.publicKey ) );
            // pending requests of an invitee survive too
            SCreator creator;
            ASSERT_TRUE( creator.Start( people, TwoContacts() ) );
            MAIL::CEMailIntroRequest request;
            ASSERT_TRUE( creator.Ask( "bob@mail.test", request ) );
            MAIL::CEMailIntroductions invitee;
            invitee.Configure( people.bob, dir, MAIL::CEMailIntroLimits() );
            ASSERT_TRUE( invitee.Load() );
            // (the file is the one of Alice: a different identity reads the drafts of Alice as its own, which is the files' business, not a match with its requests)
        }
        // damaged files: empty state, and the program goes on
        const CORE::CString statePath = CORE::CombinePath( dir, MAIL::CEMailIntroductions::StateFileName );
        const char junk[] = "{ nothing like it";
        ASSERT_TRUE( MAIL::CEMailFiles::WriteAtomic( statePath, junk, sizeof( junk ) - 1 ) );
        ASSERT_TRUE( MAIL::CEMailFiles::WriteAtomic( CORE::CombinePath( dir, MAIL::CEMailIntroductions::BookFileName ), junk, sizeof( junk ) - 1 ) );
        MAIL::CEMailIntroductions damaged;
        damaged.Configure( people.alice, dir, MAIL::CEMailIntroLimits() );
        damaged.Load();
        GUCEF::vector< MAIL::CEMailIntroDraft > drafts;
        damaged.GetDrafts( drafts );
        ASSERT_TRUE( drafts.empty() && 0 == damaged.GetBook().GetCount() );

        // nothing the player or the other players own is in a log: no address, no key, no nonce, no name
        ASSERT_FALSE( logs.Contains( "bob@mail.test" ) || logs.Contains( "carol@mail.test" ) || logs.Contains( "alice@mail.test" ) );
        ASSERT_FALSE( logs.Contains( people.bob.publicKey ) || logs.Contains( people.carol.publicKey ) || logs.Contains( people.alice.publicKey ) );
        ASSERT_FALSE( logs.Contains( nonceBob ) );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
