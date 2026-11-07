/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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
#include "TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#include "TestMatchCommon.h"
#include "TestMatchModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Three identities with keys: alice creates, bob and carol are invited */
static bool
MakeThree( GUCEF::vector< MULTIPLAY::CPlayerIdentity >& identities )
{GUCEF_TRACE;

    static const char* names[ 3 ] = { "Alice", "Bob", "Carol" };
    identities.clear();
    for ( int i=0; i<3; ++i )
    {
        MULTIPLAY::CPlayerIdentity identity;
        if ( !TESTFW::MakeTestIdentity( names[ i ], identity ) )
            return false;
        identities.push_back( identity );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Flips one character of a hex text to another valid hex digit */
static CORE::CString
FlipHex( const CORE::CString& hex, UInt32 index )
{GUCEF_TRACE;

    CORE::CString copy = hex;
    char* chars = copy.C_String();
    chars[ index ] = ( '0' == chars[ index ] ) ? '1' : '0';
    return copy;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatchModelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMULTIPLAY matches: model and signatures" );

    GUCEF_TESTFW_TESTCASE( "Test: A match id is a random lower case GUID and nothing else is accepted" )
        try
        {
            CORE::CString::StringSet seen;
            for ( int i=0; i<200; ++i )
            {
                CORE::CString id = MULTIPLAY::CMatchId::Generate();
                ASSERT_TRUE( MULTIPLAY::CMatchId::IsValid( id ) );
                ASSERT_TRUE( seen.insert( id ).second );
            }
            ASSERT_TRUE( MULTIPLAY::CMatchId::IsValid( "3f2504e0-4f89-41d3-9a0c-0305e82c3301" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "3F2504E0-4F89-41D3-9A0C-0305E82C3301" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "3f2504e04f8941d39a0c0305e82c3301" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "3f2504e0-4f89-41d3-9a0c-0305e82c330" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "3f2504e0-4f89-41d3-9a0c-0305e82c33012" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "3f2504e0-4f89-41d3-9a0c-0305e82c330g" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchId::IsValid( "../etc/passwd/../../0305e82c3301aaaaa" ) );
            ASSERT_TRUE( "3f2504e0-4f89-41d3-9a0c-0305e82c3301" == MULTIPLAY::CMatchId::Normalize( "3F2504E0-4F89-41D3-9A0C-0305E82C3301" ) );
            ASSERT_TRUE( MULTIPLAY::CMatchId::Normalize( "nope" ).IsNULLOrEmpty() );
            ASSERT_TRUE( "3f2504e0" == MULTIPLAY::CMatchId::Short( "3f2504e0-4f89-41d3-9a0c-0305e82c3301" ) );

            MULTIPLAY::CMatchId id( "3f2504e0-4f89-41d3-9a0c-0305e82c3301" );
            MULTIPLAY::CMatchId back;
            ASSERT_TRUE( MatchJsonRoundTrip( id, back, "matchid" ) && back == id && back.IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The info of a match, with participants, keys and provider settings, survives a JSON round trip" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "Friday game", false );
            info.decisionPolicy = MULTIPLAY::MATCHDECISIONPOLICY_BOSS;
            info.participants[ 1 ].boss = true;
            info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_DECLINED;
            info.participants[ 2 ].outcome = MULTIPLAY::MATCHOUTCOME_QUIT;
            info.SetSetting( "mailTransfer", "BODY_TEXT" );
            info.SetSetting( "mailDistribution", "ALL" );

            MULTIPLAY::CMatchInfo back;
            REQUIRE_TRUE( MatchJsonRoundTrip( info, back, "info" ) );
            ASSERT_TRUE( back.matchId == info.matchId && back.title == "Friday game" && back.gameId == "testgame" );
            ASSERT_TRUE( MULTIPLAY::MATCHDECISIONPOLICY_BOSS == back.decisionPolicy && MULTIPLAY::MATCHMODE_ASYNC == back.mode );
            ASSERT_TRUE( 3 == back.participants.size() );
            for ( size_t i=0; i<3; ++i )
                ASSERT_TRUE( back.participants[ i ] == info.participants[ i ] );
            ASSERT_TRUE( "BODY_TEXT" == back.GetSetting( "mailTransfer" ) && "ALL" == back.GetSetting( "mailDistribution" ) );
            ASSERT_TRUE( 2 == back.settings.size() && "fallback" == back.GetSetting( "none", "fallback" ) );
            ASSERT_TRUE( back.GetDigest() == info.GetDigest() );
            CORE::CString reason;
            ASSERT_TRUE( back.IsValid( reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The signed bytes of an info do not depend on the order settings were set, and exclude the mutable status" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo a = TESTFW::MakeTestMatchInfo( ids, "t", false );
            MULTIPLAY::CMatchInfo b = a;
            a.SetSetting( "x", "1" ); a.SetSetting( "y", "2" ); a.SetSetting( "a", "3" );
            b.SetSetting( "a", "3" ); b.SetSetting( "y", "2" ); b.SetSetting( "x", "1" );
            ASSERT_TRUE( a.GetDigest() == b.GetDigest() );
            b.status = MULTIPLAY::MATCHSTATUS_FINISHED;
            b.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
            b.participants[ 1 ].outcome = MULTIPLAY::MATCHOUTCOME_WON;
            ASSERT_TRUE( a.GetDigest() == b.GetDigest() );
            b.SetSetting( "x", "other" );
            ASSERT_FALSE( a.GetDigest() == b.GetDigest() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An info is only valid with distinct participants whose user ids follow from their keys, and a creator among them" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo good = TESTFW::MakeTestMatchInfo( ids, "t", false );
            CORE::CString reason;
            ASSERT_TRUE( good.IsValid( reason ) );

            MULTIPLAY::CMatchInfo bad = good;
            bad.matchId = "not-a-match-id";
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.participants[ 1 ].userId = ids[ 2 ].userId;                 // a user id that is not the one of the key
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.participants[ 2 ] = bad.participants[ 1 ];                   // the same participant twice
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.participants[ 2 ].slot = bad.participants[ 1 ].slot;
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.creatorUserId = "someone-else";
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.participants.clear();
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.participants[ 1 ].publicKey = "zz";
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            bad.title.Clear();
            for ( int i=0; i<200; ++i )
                bad.title += "x";
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            for ( int i=0; i<40; ++i )
                bad.SetSetting( "k" + CORE::UInt32ToString( i ), "v" );
            ASSERT_FALSE( bad.IsValid( reason ) );
            bad = good;
            for ( int i=0; i<20; ++i )
            {
                MULTIPLAY::CMatchParticipant p = good.participants[ 0 ];
                p.slot = 100 + i;
                bad.participants.push_back( p );
            }
            ASSERT_FALSE( bad.IsValid( reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A signature is bound to its kind and its match: it holds for nothing else" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            MULTIPLAY::CPlayerIdentity alice, bob;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Alice", alice ) && TESTFW::MakeTestIdentity( "Bob", bob ) );
            const CORE::CString matchA = MULTIPLAY::CMatchId::Generate();
            const CORE::CString matchB = MULTIPLAY::CMatchId::Generate();
            const char data[] = "the same record bytes";
            CORE::CString signature;
            REQUIRE_TRUE( MULTIPLAY::CMatchSigning::Sign( alice, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), signature ) );
            ASSERT_TRUE( 128 == signature.Length() );
            ASSERT_TRUE( MULTIPLAY::CMatchSigning::Verify( alice.publicKey, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), signature ) );
            // another match, another kind, another key, other bytes
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( alice.publicKey, MULTIPLAY::CMatchSigning::KindPreview, matchB, data, sizeof( data ), signature ) );
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( alice.publicKey, MULTIPLAY::CMatchSigning::KindProposal, matchA, data, sizeof( data ), signature ) );
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( bob.publicKey, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), signature ) );
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( alice.publicKey, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ) - 1, signature ) );
            // not signable or verifiable
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Sign( alice, MULTIPLAY::CMatchSigning::KindPreview, "not-an-id", data, sizeof( data ), signature ) );
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( alice.publicKey, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), "" ) );
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( alice.publicKey, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), signature.SubstrFromRange( 0, 127 ) ) );
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Verify( "short", MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), signature ) );
            MULTIPLAY::CPlayerIdentity keyless = alice.GetPublicIdentity();
            ASSERT_FALSE( MULTIPLAY::CMatchSigning::Sign( keyless, MULTIPLAY::CMatchSigning::KindPreview, matchA, data, sizeof( data ), signature ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The match record is signed by its creator and every change, wrong signer or other match is refused" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "Friday game", false );
            info.SetSetting( "mailTransfer", "ATTACHMENT" );

            MULTIPLAY::CMatchRecord record( info );
            CORE::CString reason;
            ASSERT_FALSE( record.Verify( reason ) );                    // unsigned
            ASSERT_FALSE( record.Sign( ids[ 1 ] ) );                    // bob is not the creator
            REQUIRE_TRUE( record.Sign( ids[ 0 ] ) );
            ASSERT_TRUE( record.Verify( reason ) );

            MULTIPLAY::CMatchRecord back;
            REQUIRE_TRUE( MatchJsonRoundTrip( record, back, "record" ) );
            ASSERT_TRUE( back.Verify( reason ) && back.info.GetDigest() == info.GetDigest() );

            MULTIPLAY::CMatchRecord tampered = record;
            tampered.info.title = "Saturday game";
            ASSERT_FALSE( tampered.Verify( reason ) );
            tampered = record;
            tampered.info.SetSetting( "mailTransfer", "BODY_TEXT" );
            ASSERT_FALSE( tampered.Verify( reason ) );
            tampered = record;
            tampered.info.SetSetting( "extra", "1" );
            ASSERT_FALSE( tampered.Verify( reason ) );
            tampered = record;
            tampered.info.decisionPolicy = MULTIPLAY::MATCHDECISIONPOLICY_BOSS;
            ASSERT_FALSE( tampered.Verify( reason ) );
            tampered = record;
            tampered.info.participants[ 1 ].contact = "mallory@example.invalid";
            ASSERT_FALSE( tampered.Verify( reason ) );
            tampered = record;
            tampered.info.participants[ 1 ].boss = true;
            ASSERT_FALSE( tampered.Verify( reason ) );
            tampered = record;
            tampered.signature = FlipHex( record.signature, 10 );
            ASSERT_FALSE( tampered.Verify( reason ) );

            // another key replaces a pinned key: the record was not signed for it
            MULTIPLAY::CPlayerIdentity mallory;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
            tampered = record;
            tampered.info.participants[ 2 ].publicKey = mallory.publicKey;
            tampered.info.participants[ 2 ].userId = mallory.userId;
            ASSERT_FALSE( tampered.Verify( reason ) );

            // mallory signs a record that names alice as the creator
            MULTIPLAY::CMatchRecord forged( info );
            ASSERT_FALSE( forged.Sign( mallory ) );
            forged.signature = record.signature;
            forged.info.matchId = MULTIPLAY::CMatchId::Generate();       // the signature of one match presented for another
            ASSERT_FALSE( forged.Verify( reason ) );

            // the status is state, not part of the signed record
            tampered = record;
            tampered.info.status = MULTIPLAY::MATCHSTATUS_FINISHED;
            tampered.info.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
            ASSERT_TRUE( tampered.Verify( reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An acceptance is signed by the invitee for exactly one record" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "t", false );
            MULTIPLAY::CMatchAcceptance acceptance;
            REQUIRE_TRUE( acceptance.Sign( ids[ 1 ], info, true ) );
            CORE::CString reason;
            ASSERT_TRUE( acceptance.Verify( info, reason ) && acceptance.accepted );

            MULTIPLAY::CMatchAcceptance back;
            REQUIRE_TRUE( MatchJsonRoundTrip( acceptance, back, "acceptance" ) );
            ASSERT_TRUE( back.Verify( info, reason ) );

            MULTIPLAY::CMatchAcceptance decline;
            REQUIRE_TRUE( decline.Sign( ids[ 2 ], info, false ) );
            ASSERT_TRUE( decline.Verify( info, reason ) && !decline.accepted );

            // a decline turned into an acceptance, or an acceptance of someone else, or for another record
            MULTIPLAY::CMatchAcceptance tampered = decline;
            tampered.accepted = true;
            ASSERT_FALSE( tampered.Verify( info, reason ) );
            tampered = acceptance;
            tampered.userId = ids[ 2 ].userId;
            ASSERT_FALSE( tampered.Verify( info, reason ) );
            MULTIPLAY::CMatchInfo changed = info;
            changed.SetSetting( "mailTransfer", "BODY_TEXT" );
            ASSERT_FALSE( acceptance.Verify( changed, reason ) );
            MULTIPLAY::CMatchInfo other = TESTFW::MakeTestMatchInfo( ids, "t", false );
            ASSERT_FALSE( acceptance.Verify( other, reason ) );
            // the identity of the invitee has to be the pinned one
            MULTIPLAY::CPlayerIdentity mallory;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
            MULTIPLAY::CMatchAcceptance forged;
            ASSERT_FALSE( forged.Sign( mallory, info, true ) );
            // the state of the participants does not change what an acceptance is bound to
            changed = info;
            changed.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
            ASSERT_TRUE( acceptance.Verify( changed, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A preview is signed and any change, wrong key or other match breaks it; it is under 1 KB" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "t", true );
            MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "first package" );
            MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, ids[ 0 ], package, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 123456 );
            preview.deadline = 999999;
            REQUIRE_TRUE( preview.Sign( ids[ 0 ] ) );

            ASSERT_TRUE( preview.VerifySignature() && preview.VerifyWithKey( ids[ 0 ].publicKey ) );
            ASSERT_TRUE( ids[ 0 ].userId == preview.GetIssuerUserId() );
            ASSERT_FALSE( preview.VerifyWithKey( ids[ 1 ].publicKey ) );

            MULTIPLAY::CMatchPreview back;
            REQUIRE_TRUE( MatchJsonRoundTrip( preview, back, "preview" ) );
            ASSERT_TRUE( back.VerifySignature() && back.revision == 1 && back.IsCurrent( ids[ 1 ].userId ) && 999999 == back.deadline );

            MULTIPLAY::CMatchPreview t = preview;
            t.revision = 2; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.turn = 9; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.currentUserIds.insert( ids[ 2 ].userId ); ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.currentUserIds.clear(); t.currentUserIds.insert( ids[ 2 ].userId ); ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.status = MULTIPLAY::MATCHSTATUS_FINISHED; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.deadline = 1; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.packageSha256 = FlipHex( preview.packageSha256, 3 ); ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.summary = "cheat"; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.lastActivity = 5; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.signature = FlipHex( preview.signature, 0 ); ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.signature = FlipHex( preview.signature, 127 ); ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.issuerKey = ids[ 1 ].publicKey; ASSERT_FALSE( t.VerifySignature() );
            t = preview; t.signature.Clear(); ASSERT_FALSE( t.VerifySignature() );

            // the match id is the context: the same fields signed for another match give another signature, and one cannot be moved
            t = preview;
            t.matchId = MULTIPLAY::CMatchId::Generate();
            ASSERT_FALSE( t.VerifySignature() );
            MULTIPLAY::CMatchPreview other = preview;
            other.matchId = t.matchId;
            REQUIRE_TRUE( other.Sign( ids[ 0 ] ) );
            ASSERT_TRUE( other.VerifySignature() && other.signature != preview.signature );

            // without a key pair there is nothing to sign with
            MULTIPLAY::CMatchPreview unsigned1 = preview;
            ASSERT_FALSE( unsigned1.Sign( ids[ 0 ].GetPublicIdentity() ) );
            ASSERT_FALSE( unsigned1.VerifySignature() );

            // the binary form, which is what is shipped, is under 1 KB even with the limits used up
            CORE::CDynamicBuffer bytes;
            REQUIRE_TRUE( preview.ToBinary( bytes ) );
            ASSERT_TRUE( bytes.GetDataSize() < 1024 );
            MULTIPLAY::CMatchPreview parsed;
            ASSERT_TRUE( parsed.FromBinary( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) && parsed.VerifySignature() );
            ASSERT_TRUE( parsed.matchId == preview.matchId && parsed.currentUserIds == preview.currentUserIds && parsed.summary == "turn" );

            MULTIPLAY::CMatchPreview max = preview;
            max.summary = "";
            for ( UInt32 i=0; i<MULTIPLAY::CMatchPreview::MaxSummaryLength; ++i )
                max.summary += "s";
            max.currentUserIds.clear();
            for ( UInt32 i=0; i<MULTIPLAY::CMatchPreview::MaxCurrentUserIds; ++i )
                max.currentUserIds.insert( MULTIPLAY::CMatchId::Generate() );
            REQUIRE_TRUE( max.Sign( ids[ 0 ] ) );
            CORE::CDynamicBuffer maxBytes;
            ASSERT_TRUE( max.ToBinary( maxBytes ) && maxBytes.GetDataSize() < 1024 );
            MULTIPLAY::CMatchPreview tooMuch = max;
            tooMuch.summary += "s";
            REQUIRE_TRUE( tooMuch.Sign( ids[ 0 ] ) == true );
            ASSERT_FALSE( tooMuch.IsWithinLimits() );
            ASSERT_FALSE( tooMuch.VerifySignature() );
            CORE::CDynamicBuffer none;
            ASSERT_FALSE( tooMuch.ToBinary( none ) );
            tooMuch = max;
            tooMuch.currentUserIds.insert( MULTIPLAY::CMatchId::Generate() );
            ASSERT_FALSE( tooMuch.IsWithinLimits() );

            // every shorter piece of the bytes, and bytes behind the end, are no preview
            for ( UInt32 n=0; n<bytes.GetDataSize(); ++n )
            {
                MULTIPLAY::CMatchPreview piece;
                ASSERT_FALSE( piece.FromBinary( bytes.GetConstBufferPtr(), n ) );
            }
            CORE::CDynamicBuffer longer = bytes;
            UInt8 extra = 0;
            longer.Append( &extra, 1 );
            MULTIPLAY::CMatchPreview piece;
            ASSERT_FALSE( piece.FromBinary( longer.GetConstBufferPtr(), longer.GetDataSize() ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Bytes of a preview that were changed never give a different valid preview" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "t", true );
            MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "first package" );
            MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, ids[ 0 ], package, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 123456 );
            CORE::CDynamicBuffer bytes;
            REQUIRE_TRUE( preview.ToBinary( bytes ) );
            UInt32 accepted = 0;
            for ( UInt32 i=0; i<bytes.GetDataSize(); ++i )
            {
                for ( int bit=0; bit<8; ++bit )
                {
                    CORE::CDynamicBuffer copy = bytes;
                    copy.GetUByte( i ) ^= (UInt8)( 1 << bit );
                    MULTIPLAY::CMatchPreview parsed;
                    if ( parsed.FromBinary( copy.GetConstBufferPtr(), copy.GetDataSize() ) && parsed.VerifySignature() )
                        ++accepted;
                }
            }
            ASSERT_TRUE( 0 == accepted );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A package is its payload and hashes and the chain checks the signer, the turn, the revision and the base" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "t", true );
            CORE::CString reason;

            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game state one" );
            ASSERT_TRUE( p1.IsConsistent() && 64 == p1.sha256.Length() );
            MULTIPLAY::CMatchPackage backPackage;
            REQUIRE_TRUE( MatchJsonRoundTrip( p1, backPackage, "package" ) );
            ASSERT_TRUE( backPackage.IsConsistent() && backPackage.payload == p1.payload && 1 == backPackage.revision && backPackage.baseSha256.IsNULLOrEmpty() );
            MULTIPLAY::CMatchPackage damaged = p1;
            damaged.payload.GetUByte( 0 ) ^= 1;
            ASSERT_FALSE( damaged.IsConsistent() );

            // revision 1 by the creator
            MULTIPLAY::CMatchPreview r1 = TESTFW::MakeTestPreview( info, ids[ 0 ], p1, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OK == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, GUCEF_NULL, r1, &p1, reason ) );
            MULTIPLAY::CMatchPreview byBob = TESTFW::MakeTestPreview( info, ids[ 1 ], p1, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_NOT_ON_TURN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, GUCEF_NULL, byBob, &p1, reason ) );

            // revision 2 by bob, whose turn it was
            MULTIPLAY::CMatchPackage p2 = TESTFW::MakeTestPackage( 2, &p1, "game state two" );
            MULTIPLAY::CMatchPreview r2 = TESTFW::MakeTestPreview( info, ids[ 1 ], p2, &ids[ 2 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 2000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OK == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r1, r2, &p2, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OK == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r1, r2, GUCEF_NULL, reason ) );

            // not signed by the participant whose turn it was
            MULTIPLAY::CMatchPreview byCarol = TESTFW::MakeTestPreview( info, ids[ 2 ], p2, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 2000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_NOT_ON_TURN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r1, byCarol, &p2, reason ) );
            // by a key that is not pinned
            MULTIPLAY::CPlayerIdentity mallory;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
            MULTIPLAY::CMatchPreview byMallory = TESTFW::MakeTestPreview( info, mallory, p2, &ids[ 2 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 2000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_UNKNOWN_SIGNER == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r1, byMallory, &p2, reason ) );
            // a preview that was changed after signing
            MULTIPLAY::CMatchPreview changed = r2;
            changed.summary = "changed";
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BAD_PREVIEW == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r1, changed, &p2, reason ) );
            // an old package replayed
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OLD_REVISION == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r1, &p1, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OLD_REVISION == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r2, &p2, reason ) );
            // a revision that skips one
            MULTIPLAY::CMatchPackage p3 = TESTFW::MakeTestPackage( 3, &p2, "game state three" );
            MULTIPLAY::CMatchPackage p4 = TESTFW::MakeTestPackage( 4, &p3, "game state four" );
            MULTIPLAY::CMatchPreview r4 = TESTFW::MakeTestPreview( info, ids[ 2 ], p4, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 4000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BROKEN_CHAIN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r4, &p4, reason ) );
            // a package that does not follow the trusted one
            MULTIPLAY::CMatchPackage wrongBase = TESTFW::MakeTestPackage( 3, &p1, "game state three" );
            MULTIPLAY::CMatchPreview r3wrong = TESTFW::MakeTestPreview( info, ids[ 2 ], wrongBase, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 3000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BROKEN_CHAIN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r3wrong, &wrongBase, reason ) );
            // a package that is not the one the signed preview names
            MULTIPLAY::CMatchPreview r3 = TESTFW::MakeTestPreview( info, ids[ 2 ], p3, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 3000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OK == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r3, &p3, reason ) );
            MULTIPLAY::CMatchPackage swapped = TESTFW::MakeTestPackage( 3, &p2, "something else" );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BAD_PACKAGE == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r3, &swapped, reason ) );
            MULTIPLAY::CMatchPackage corrupt = p3;
            corrupt.payload.GetUByte( 1 ) ^= 0x10;
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BAD_PACKAGE == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, &r2, r3, &corrupt, reason ) );
            // another match
            MULTIPLAY::CMatchInfo otherInfo = TESTFW::MakeTestMatchInfo( ids, "other", true );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_WRONG_MATCH == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( otherInfo, &r1, r2, &p2, reason ) );
            // a first revision that is not revision 1
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BROKEN_CHAIN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( info, GUCEF_NULL, TESTFW::MakeTestPreview( info, ids[ 0 ], p2, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1 ), &p2, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two different signed successors of one revision are proof that the signer equivocated" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "t", true );
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "one" );
            MULTIPLAY::CMatchPackage p2a = TESTFW::MakeTestPackage( 2, &p1, "two A" );
            MULTIPLAY::CMatchPackage p2b = TESTFW::MakeTestPackage( 2, &p1, "two B" );
            MULTIPLAY::CMatchPreview a = TESTFW::MakeTestPreview( info, ids[ 1 ], p2a, &ids[ 2 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1 );
            MULTIPLAY::CMatchPreview b = TESTFW::MakeTestPreview( info, ids[ 1 ], p2b, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1 );
            ASSERT_TRUE( MULTIPLAY::CMatchChainVerifier::IsEquivocation( a, b ) );
            ASSERT_FALSE( MULTIPLAY::CMatchChainVerifier::IsEquivocation( a, a ) );
            // another signer, another revision or a forged half is no proof
            MULTIPLAY::CMatchPreview byCarol = TESTFW::MakeTestPreview( info, ids[ 2 ], p2b, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1 );
            ASSERT_FALSE( MULTIPLAY::CMatchChainVerifier::IsEquivocation( a, byCarol ) );
            MULTIPLAY::CMatchPackage p3 = TESTFW::MakeTestPackage( 3, &p2a, "three" );
            ASSERT_FALSE( MULTIPLAY::CMatchChainVerifier::IsEquivocation( a, TESTFW::MakeTestPreview( info, ids[ 1 ], p3, &ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1 ) ) );
            MULTIPLAY::CMatchPreview forged = b;
            forged.summary = "forged";
            ASSERT_FALSE( MULTIPLAY::CMatchChainVerifier::IsEquivocation( a, forged ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A match entry keeps its local state through a JSON round trip" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "Friday game", true );
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "one" );
            MULTIPLAY::CMatchEntry entry;
            entry.preview = TESTFW::MakeTestPreview( info, ids[ 0 ], p1, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 77 );
            entry.info = info;
            entry.hasInfo = true;
            entry.providerName = "fake";
            entry.lastSeenRevision = 4;
            entry.unseen = true;
            entry.muted = true;
            entry.pinned = true;
            entry.archived = true;
            entry.cachedPackagePath = "cache/x.pkg";
            entry.yourTurnSince = 123456789;
            entry.notifiedRevision = 6;
            entry.cursor = "uidvalidity=7;uid=42";

            MULTIPLAY::CMatchEntry back;
            REQUIRE_TRUE( MatchJsonRoundTrip( entry, back, "entry" ) );
            ASSERT_TRUE( back.GetMatchId() == info.matchId && back.hasInfo && back.info.GetDigest() == info.GetDigest() );
            ASSERT_TRUE( "fake" == back.providerName && 4 == back.lastSeenRevision && back.unseen && back.muted && back.pinned && back.archived );
            ASSERT_TRUE( "cache/x.pkg" == back.cachedPackagePath && 123456789 == back.yourTurnSince && 6 == back.notifiedRevision );
            ASSERT_TRUE( "uidvalidity=7;uid=42" == back.cursor && back.preview.VerifySignature() );

            entry.hasInfo = false;
            REQUIRE_TRUE( MatchJsonRoundTrip( entry, back, "entry2" ) );
            ASSERT_FALSE( back.hasInfo );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A match entry has the player's own title for display, kept by copies, clones and the data node, and old records without it load" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            REQUIRE_TRUE( MakeThree( ids ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "Shared title", true );
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "one" );
            MULTIPLAY::CMatchEntry entry;
            entry.preview = TESTFW::MakeTestPreview( info, ids[ 0 ], p1, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 77 );

            // the display title: the player's own, else the shared one when the info is known, else nothing
            ASSERT_TRUE( entry.localTitle.IsNULLOrEmpty() && entry.GetDisplayTitle().IsNULLOrEmpty() );
            entry.info = info;
            ASSERT_TRUE( entry.GetDisplayTitle().IsNULLOrEmpty() );     // the info is not known yet
            entry.hasInfo = true;
            ASSERT_TRUE( "Shared title" == entry.GetDisplayTitle() );
            entry.localTitle = "Mine";
            ASSERT_TRUE( "Mine" == entry.GetDisplayTitle() );
            entry.hasInfo = false;
            ASSERT_TRUE( "Mine" == entry.GetDisplayTitle() );
            entry.hasInfo = true;
            entry.localTitle.Clear();
            entry.info.title.Clear();
            ASSERT_TRUE( entry.GetDisplayTitle().IsNULLOrEmpty() );
            entry.info.title = "Shared title";
            entry.localTitle = "Mine";

            // copies, assignment and clones carry it
            MULTIPLAY::CMatchEntry copy( entry );
            ASSERT_TRUE( "Mine" == copy.localTitle );
            MULTIPLAY::CMatchEntry assigned;
            assigned = entry;
            ASSERT_TRUE( "Mine" == assigned.localTitle );
            MULTIPLAY::CMatchEntry* cloned = static_cast< MULTIPLAY::CMatchEntry* >( entry.Clone() );
            REQUIRE_TRUE( GUCEF_NULL != cloned );
            ASSERT_TRUE( "Mine" == cloned->localTitle );
            GUCEF_DELETE cloned;

            // the data node carries it, and the shared info is not changed by it
            CORE::CDataNode node( "Entry" );
            CORE::CDataNodeSerializableSettings settings;
            REQUIRE_TRUE( entry.Serialize( node, settings ) );
            ASSERT_TRUE( node.IsAttribute( "localTitle" ) && "Mine" == node.GetAttributeValue( "localTitle" ).AsString() );
            MULTIPLAY::CMatchEntry back;
            REQUIRE_TRUE( back.Deserialize( node, settings ) );
            ASSERT_TRUE( "Mine" == back.localTitle && "Shared title" == back.info.title );

            // the JSON route
            MULTIPLAY::CMatchEntry viaJson;
            REQUIRE_TRUE( MatchJsonRoundTrip( entry, viaJson, "entrytitle" ) );
            ASSERT_TRUE( "Mine" == viaJson.localTitle && "Mine" == viaJson.GetDisplayTitle() && "Shared title" == viaJson.info.title );

            // a record written before the field existed: no title, and a value left in a reused object is cleared
            node.DelAttribute( "localTitle" );
            ASSERT_FALSE( node.IsAttribute( "localTitle" ) );
            MULTIPLAY::CMatchEntry old;
            old.localTitle = "stale";
            REQUIRE_TRUE( old.Deserialize( node, settings ) );
            ASSERT_TRUE( old.localTitle.IsNULLOrEmpty() && "Shared title" == old.GetDisplayTitle() );

            // an empty title is not written as a value that reads back as something else
            entry.localTitle.Clear();
            CORE::CDataNode emptyNode( "Entry" );
            REQUIRE_TRUE( entry.Serialize( emptyNode, settings ) );
            MULTIPLAY::CMatchEntry emptyBack;
            emptyBack.localTitle = "stale";
            REQUIRE_TRUE( emptyBack.Deserialize( emptyNode, settings ) );
            ASSERT_TRUE( emptyBack.localTitle.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}
