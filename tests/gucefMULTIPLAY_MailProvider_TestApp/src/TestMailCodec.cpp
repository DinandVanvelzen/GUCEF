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

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#include "matchprovidermail_CEMailMatchCodec.h"
#include "TestMailPump.h"
#include "TestMailCodec.h"

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

CORE::CDynamicBuffer
MakeBytes( UInt32 size, UInt32 seed )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    UInt32 state = seed;
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1664525u + 1013904223u;
        UInt8 b = (UInt8)( state >> 24 );
        bytes.Append( &b, 1 );
    }
    return bytes;
}

/*-------------------------------------------------------------------------*/

bool
SameBytes( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{GUCEF_TRACE;

    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

MAIL::CEMailOutgoing
MakeOutgoing( const CORE::CString& matchId, MAIL::TMailMatchKind kind, MAIL::TMailTransfer transfer, const CORE::CDynamicBuffer* data )
{GUCEF_TRACE;

    MAIL::CEMailOutgoing outgoing;
    outgoing.kind = kind;
    outgoing.matchId = matchId;
    outgoing.revision = 3;
    outgoing.turn = 3;
    outgoing.title = "Friday game";
    outgoing.from = COM::CEMailAddress( "Alice", "alice@mail.test" );
    outgoing.to.push_back( COM::CEMailAddress( "Bob", "bob@mail.test" ) );
    outgoing.cc.push_back( COM::CEMailAddress( "Carol", "carol@mail.test" ) );
    outgoing.actorName = "Alice";
    outgoing.nextName = "Bob";
    outgoing.linkText = "Open the game: https://games.example.invalid/match/" + matchId;
    outgoing.transfer = transfer;
    outgoing.data = data;
    outgoing.dataLabel = MAIL::CEMailMatchCodec::ArmorLabelTurn;
    outgoing.dataRevisionName = "r3";
    return outgoing;
}

/*-------------------------------------------------------------------------*/

/** The message as it goes over the wire and comes back out of a parser */
bool
ThroughTheWire( const COM::CEMailMessage& message, COM::CEMailMessage& received, CORE::CString* wireText = GUCEF_NULL )
{GUCEF_TRACE;

    COM::CMimeComposer composer;
    CORE::CDynamicBuffer wire;
    CORE::CString error;
    if ( !composer.Compose( message, wire, error ) )
        return false;
    if ( GUCEF_NULL != wireText )
        *wireText = CORE::CString( static_cast< const char* >( wire.GetConstBufferPtr() ), wire.GetDataSize() );
    COM::CMimeParser parser;
    return parser.ParseMessage( wire.GetConstBufferPtr(), wire.GetDataSize(), received );
}

/*-------------------------------------------------------------------------*/

bool
IsAutomaticWith( const char* name, const char* value, const char* from = "bob@mail.test", const char* contentType = "text/plain" )
{GUCEF_TRACE;

    CORE::CString text = CORE::CString( "From: <" ) + from + ">\r\nTo: <alice@mail.test>\r\nSubject: x\r\nMessage-ID: <a@b>\r\nContent-Type: " + contentType + "\r\n";
    if ( GUCEF_NULL != name )
        text += CORE::CString( name ) + ": " + value + "\r\n";
    text += "\r\nbody\r\n";
    COM::CMimeParser parser;
    COM::CEMailHeaders headers;
    UInt32 offset = 0;
    if ( !parser.ParseHeaders( text.C_String(), text.ByteSize() - 1, headers, offset ) )
        return false;
    return MAIL::CEMailMatchCodec::IsAutomatic( headers );
}

/*-------------------------------------------------------------------------*/

CORE::CString
Quote( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString quoted;
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
        quoted += "> " + lines[ i ] + "\r\n";
    return quoted;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: the mails of a match" );

    GUCEF_TESTFW_TESTCASE( "Codec: kinds, transfers and distributions have stable names and unknown text is refused" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: kinds, transfers and distributions have stable names and unknown text is refused" ), "not the case that was asked for" );
    try
    {
        const char* names[] = { "INVITE", "ACCEPT", "DECLINE", "TURN", "ACK", "NUDGE", "RESIGN", "FINISH", "RESEND", "PROPOSAL", "VOTE", "INTRO_REQUEST", "INTRO_REPLY", "CANCEL", "INCOMPATIBLE", "ROSTER", "DRAFT_PROPOSAL", "DRAFT_VOTE" };
        for ( int i=0; i<18; ++i )
        {
            MAIL::TMailMatchKind kind = MAIL::MAILKIND_UNKNOWN;
            ASSERT_TRUE( MAIL::TryStringToMailMatchKind( names[ i ], kind ) && (int) kind == i );
            ASSERT_TRUE( CORE::CString( names[ i ] ) == MAIL::MailMatchKindToString( kind ) );
            ASSERT_TRUE( MAIL::TryStringToMailMatchKind( CORE::CString( names[ i ] ).Lowercase(), kind ) && (int) kind == i );
        }
        MAIL::TMailMatchKind unknown = MAIL::MAILKIND_TURN;
        ASSERT_FALSE( MAIL::TryStringToMailMatchKind( "TURN; DROP", unknown ) );
        ASSERT_TRUE( MAIL::MAILKIND_UNKNOWN == unknown );
        ASSERT_FALSE( MAIL::TryStringToMailMatchKind( "", unknown ) );

        MAIL::TMailTransfer transfer = MAIL::MAILTRANSFER_ATTACHMENT;
        ASSERT_TRUE( MAIL::TryStringToMailTransfer( "BODY_TEXT", transfer ) && MAIL::MAILTRANSFER_BODY_TEXT == transfer );
        ASSERT_TRUE( MAIL::TryStringToMailTransfer( "attachment", transfer ) && MAIL::MAILTRANSFER_ATTACHMENT == transfer );
        ASSERT_FALSE( MAIL::TryStringToMailTransfer( "pigeon", transfer ) );
        ASSERT_TRUE( CORE::CString( "BODY_TEXT" ) == MAIL::MailTransferToString( MAIL::MAILTRANSFER_BODY_TEXT ) );

        MAIL::TMailDistribution distribution = MAIL::MAILDISTRIBUTION_NEXT_ONLY;
        ASSERT_TRUE( MAIL::TryStringToMailDistribution( "ALL", distribution ) && MAIL::MAILDISTRIBUTION_ALL == distribution );
        ASSERT_TRUE( MAIL::TryStringToMailDistribution( "next_only", distribution ) && MAIL::MAILDISTRIBUTION_NEXT_ONLY == distribution );
        ASSERT_FALSE( MAIL::TryStringToMailDistribution( "everyone", distribution ) );
        ASSERT_TRUE( CORE::CString( "NEXT_ONLY" ) == MAIL::MailDistributionToString( MAIL::MAILDISTRIBUTION_NEXT_ONLY ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the introduction mails have their own kinds, a readable text of several lines that cannot add a header, and the data of the statement" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: the introduction mails have their own kinds, a readable text of several lines that cannot add a header, and the data of the statement" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        CORE::CDynamicBuffer data = MakeBytes( 600, 7 );
        for ( int variant=0; variant<2; ++variant )
        {
            MAIL::CEMailOutgoing request = MakeOutgoing( id, MAIL::MAILKIND_INTRO_REQUEST, variant ? MAIL::MAILTRANSFER_BODY_TEXT : MAIL::MAILTRANSFER_ATTACHMENT, &data );
            request.revision = 0;
            request.turn = 0;
            request.cc.clear();
            request.dataLabel = MAIL::CEMailMatchCodec::ArmorLabelData;
            request.dataRevisionName = "intro";
            request.bodyText = "First line of the explanation.\r\nSecond line.\r\nX-Injected: yes\r\n\r\nBcc: mallory@mail.test";
            COM::CEMailMessage message;
            CORE::CString reason;
            ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( request, message, reason ) );
            COM::CEMailMessage received;
            CORE::CString wire;
            ASSERT_TRUE( ThroughTheWire( message, received, &wire ) );
            MAIL::CEMailIncoming incoming;
            MAIL::CEMailMatchCodec::ReadHeaders( received.headers, incoming );
            ASSERT_TRUE( incoming.valid && MAIL::MAILKIND_INTRO_REQUEST == incoming.kind && incoming.matchId == id && 0 == incoming.revision );
            ASSERT_TRUE( wire.HasSubstr( "X-Redux-Kind: INTRO_REQUEST\r\n" ) >= 0 && wire.HasSubstr( "invites you" ) >= 0 );
            // several lines come through as lines of the body, and nothing in them is a header or a recipient
            ASSERT_TRUE( received.textBody.HasSubstr( "First line of the explanation." ) >= 0 && received.textBody.HasSubstr( "Second line." ) >= 0 );
            ASSERT_TRUE( !received.headers.Has( "X-Injected" ) && !received.headers.Has( "Bcc" ) && 1 == received.to.size() && received.cc.empty() );
            CORE::CDynamicBuffer back;
            MAIL::TMailTransfer used;
            ASSERT_TRUE( MAIL::MAILPAYLOAD_OK == MAIL::CEMailMatchCodec::ExtractData( received, id, 0, MAIL::CEMailMatchCodec::ArmorLabelData, 16 * 1024, 64 * 1024, back, used ) && SameBytes( data, back ) );
        }
        // too long a text is cut, not refused, and does not grow without a bound
        MAIL::CEMailOutgoing big = MakeOutgoing( id, MAIL::MAILKIND_INTRO_REPLY, MAIL::MAILTRANSFER_ATTACHMENT, &data );
        big.dataRevisionName = "intro";
        big.dataLabel = MAIL::CEMailMatchCodec::ArmorLabelData;
        for ( int i=0; i<500; ++i )
            big.bodyText += "A line of the text of a mail that goes on and on and on.\r\n";
        COM::CEMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( big, message, reason ) && message.textBody.ByteSize() < 12 * 1024 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the transfer and distribution of a match come from its settings, attachment and all by default" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: the transfer and distribution of a match come from its settings, attachment and all by default" ), "not the case that was asked for" );
    try
    {
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( MAIL::MAILTRANSFER_ATTACHMENT == MAIL::CEMailMatchCodec::GetTransfer( info ) );
        ASSERT_TRUE( MAIL::MAILDISTRIBUTION_ALL == MAIL::CEMailMatchCodec::GetDistribution( info ) );
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER, "BODY_TEXT" );
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION, "NEXT_ONLY" );
        ASSERT_TRUE( MAIL::MAILTRANSFER_BODY_TEXT == MAIL::CEMailMatchCodec::GetTransfer( info ) );
        ASSERT_TRUE( MAIL::MAILDISTRIBUTION_NEXT_ONLY == MAIL::CEMailMatchCodec::GetDistribution( info ) );
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER, "nonsense" );
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION, "" );
        ASSERT_TRUE( MAIL::MAILTRANSFER_ATTACHMENT == MAIL::CEMailMatchCodec::GetTransfer( info ) );
        ASSERT_TRUE( MAIL::MAILDISTRIBUTION_ALL == MAIL::CEMailMatchCodec::GetDistribution( info ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: a title cannot end the subject line or add a header, and is shortened on a character boundary" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: a title cannot end the subject line or add a header, and is shortened on a character boundary" ), "not the case that was asked for" );
    try
    {
        ASSERT_TRUE( CORE::CString( "Friday game" ) == MAIL::CEMailMatchCodec::SanitizeTitle( "Friday game" ) );
        const CORE::CString injected = MAIL::CEMailMatchCodec::SanitizeTitle( "Game\r\nBcc: evil@mail.test\r\n\r\nbody" );
        ASSERT_TRUE( injected.HasChar( '\r' ) < 0 && injected.HasChar( '\n' ) < 0 );
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SanitizeTitle( CORE::CString( "a\0b", 3 ) ).HasChar( '\0' ) < 0 );
        ASSERT_TRUE( CORE::CString( "tab separated" ) == MAIL::CEMailMatchCodec::SanitizeTitle( "tab\tseparated" ) );
        CORE::CString longAscii;
        for ( int i=0; i<300; ++i )
            longAscii += "x";
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SanitizeTitle( longAscii ).ByteSize() - 1 == MAIL::CEMailMatchCodec::MaxSubjectTitleBytes );
        // 3 byte characters: the cut never leaves half of one
        CORE::CString long3;
        for ( int i=0; i<60; ++i )
            long3 += "\xE2\x82\xAC";
        const CORE::CString cut = MAIL::CEMailMatchCodec::SanitizeTitle( long3 );
        ASSERT_TRUE( cut.ByteSize() - 1 <= MAIL::CEMailMatchCodec::MaxSubjectTitleBytes && 0 == ( cut.ByteSize() - 1 ) % 3 && cut.Length() > 0 );
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SanitizeTitle( "   " ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: subject and attachment names follow the format and are made from the match id only" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: subject and attachment names follow the format and are made from the match id only" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = "3f2504e0-4f89-41d3-9a0c-0305e82c3301";
        ASSERT_TRUE( CORE::CString( "[Redux 3f2504e0 r3 t4] Friday game" ) == MAIL::CEMailMatchCodec::MakeSubject( id, 3, 4, "Friday game" ) );
        ASSERT_TRUE( CORE::CString( "[Redux 3f2504e0 r1 t0]" ) == MAIL::CEMailMatchCodec::MakeSubject( id, 1, 0, "" ).Trim( false ) );
        ASSERT_TRUE( MAIL::CEMailMatchCodec::MakeSubject( id, 3, 4, "x\r\nBcc: a@b" ).HasChar( '\n' ) < 0 );
        ASSERT_TRUE( CORE::CString( "redux-3f2504e0-r3.rdxturn" ) == MAIL::CEMailMatchCodec::MakeAttachmentName( id, "r3" ) );
        ASSERT_TRUE( CORE::CString( "redux-3f2504e0-record.rdxdata" ) == MAIL::CEMailMatchCodec::MakeAttachmentName( id, "record" ) );
        ASSERT_TRUE( MAIL::CEMailMatchCodec::MakeAttachmentName( "../../etc/passwd", "r3" ).IsNULLOrEmpty() );
        ASSERT_TRUE( MAIL::CEMailMatchCodec::MakeAttachmentName( id, "../x" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: an ATTACHMENT mail has the headers, the readable text and the attachment, and the data comes back byte for byte" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: an ATTACHMENT mail has the headers, the readable text and the attachment, and the data comes back byte for byte" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        CORE::CDynamicBuffer data = MakeBytes( 3000, 11 );
        MAIL::CEMailOutgoing outgoing = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_ATTACHMENT, &data );
        outgoing.inReplyTo = "<previous@mail.test>";
        outgoing.references.push_back( "<first@mail.test>" );
        outgoing.references.push_back( "<previous@mail.test>" );
        outgoing.packageSha256 = "ab12";
        COM::CEMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( outgoing, message, reason ) );

        COM::CEMailMessage received;
        CORE::CString wire;
        ASSERT_TRUE( ThroughTheWire( message, received, &wire ) );
        ASSERT_TRUE( received.subject == MAIL::CEMailMatchCodec::MakeSubject( id, 3, 3, "Friday game" ) );
        ASSERT_TRUE( received.headers.Get( MAIL::CEMailMatchCodec::HeaderMatch ) == id );
        ASSERT_TRUE( received.headers.Get( MAIL::CEMailMatchCodec::HeaderRevision ) == "3" );
        ASSERT_TRUE( received.headers.Get( MAIL::CEMailMatchCodec::HeaderKind ) == "TURN" );
        ASSERT_TRUE( received.headers.Get( MAIL::CEMailMatchCodec::HeaderProtocol ) == "1" );
        ASSERT_TRUE( received.inReplyTo.size() == 1 && received.inReplyTo[ 0 ] == "<previous@mail.test>" && received.references.size() == 2 );
        ASSERT_TRUE( 1 == received.to.size() && 1 == received.cc.size() && "bob@mail.test" == received.to[ 0 ].GetAddress() && "carol@mail.test" == received.cc[ 0 ].GetAddress() );
        ASSERT_TRUE( 1 == received.attachments.size() && "application/octet-stream" == received.attachments[ 0 ].contentType.Lowercase() );
        ASSERT_TRUE( received.attachments[ 0 ].fileName == MAIL::CEMailMatchCodec::MakeAttachmentName( id, "r3" ) );
        // a person can read it without the game, and no encoded data is in the text
        ASSERT_TRUE( received.textBody.HasSubstr( "Friday game" ) >= 0 && received.textBody.HasSubstr( "Alice" ) >= 0 && received.textBody.HasSubstr( "Bob" ) >= 0 );
        ASSERT_TRUE( received.textBody.HasSubstr( "https://games.example.invalid/match/" + id ) >= 0 );
        ASSERT_TRUE( received.textBody.HasSubstr( "BEGIN" ) < 0 );

        MAIL::CEMailIncoming incoming;
        MAIL::CEMailMatchCodec::ReadHeaders( received.headers, incoming );
        ASSERT_TRUE( incoming.valid && MAIL::MAILKIND_TURN == incoming.kind && incoming.matchId == id && 3 == incoming.revision && 1 == incoming.protocolVersion && !incoming.automatic );
        ASSERT_TRUE( incoming.fromAddress == "alice@mail.test" && incoming.packageSha256 == "ab12" );

        CORE::CDynamicBuffer extracted;
        MAIL::TMailTransfer used = MAIL::MAILTRANSFER_BODY_TEXT;
        ASSERT_TRUE( MAIL::MAILPAYLOAD_OK == MAIL::CEMailMatchCodec::ExtractData( received, id, 3, MAIL::CEMailMatchCodec::ArmorLabelTurn, 2 * 1024 * 1024, 512 * 1024, extracted, used ) );
        ASSERT_TRUE( SameBytes( data, extracted ) && MAIL::MAILTRANSFER_ATTACHMENT == used );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: a BODY_TEXT mail carries the data as armored text in the body, with no attachment, and it comes back byte for byte" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: a BODY_TEXT mail carries the data as armored text in the body, with no attachment, and it comes back byte for byte" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        CORE::CDynamicBuffer data = MakeBytes( 4000, 12 );
        MAIL::CEMailOutgoing outgoing = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_BODY_TEXT, &data );
        COM::CEMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( outgoing, message, reason ) );
        COM::CEMailMessage received;
        ASSERT_TRUE( ThroughTheWire( message, received ) );
        ASSERT_TRUE( received.attachments.empty() );
        ASSERT_TRUE( received.textBody.HasSubstr( "-----BEGIN MATCH TURN-----" ) >= 0 && received.textBody.HasSubstr( "-----END MATCH TURN-----" ) >= 0 );
        ASSERT_TRUE( received.textBody.HasSubstr( "Friday game" ) >= 0 );

        CORE::CDynamicBuffer extracted;
        MAIL::TMailTransfer used = MAIL::MAILTRANSFER_ATTACHMENT;
        ASSERT_TRUE( MAIL::MAILPAYLOAD_OK == MAIL::CEMailMatchCodec::ExtractData( received, id, 3, MAIL::CEMailMatchCodec::ArmorLabelTurn, 2 * 1024 * 1024, 512 * 1024, extracted, used ) );
        ASSERT_TRUE( SameBytes( data, extracted ) && MAIL::MAILTRANSFER_BODY_TEXT == used );

        // a reply that quotes the text still carries it
        COM::CEMailMessage reply = received;
        reply.attachments.clear();
        reply.textBody = "Thanks, my move follows.\r\n\r\n" + Quote( received.textBody );
        CORE::CDynamicBuffer fromReply;
        ASSERT_TRUE( MAIL::MAILPAYLOAD_OK == MAIL::CEMailMatchCodec::ExtractData( reply, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 2 * 1024 * 1024, 512 * 1024, fromReply, used ) );
        ASSERT_TRUE( SameBytes( data, fromReply ) );
        // the text of another match is no answer
        ASSERT_TRUE( MAIL::MAILPAYLOAD_NONE == MAIL::CEMailMatchCodec::ExtractData( received, MULTIPLAY::CMatchId::Generate(), 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 2 * 1024 * 1024, 512 * 1024, fromReply, used ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: text from outside that is damaged, too large, missing or hostile is refused with the right result" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: text from outside that is damaged, too large, missing or hostile is refused with the right result" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        CORE::CDynamicBuffer data = MakeBytes( 2000, 13 );
        CORE::CDynamicBuffer extracted;
        MAIL::TMailTransfer used = MAIL::MAILTRANSFER_ATTACHMENT;

        // nothing in the mail
        COM::CEMailMessage plain;
        plain.textBody = "Just a note";
        ASSERT_TRUE( MAIL::MAILPAYLOAD_NONE == MAIL::CEMailMatchCodec::ExtractData( plain, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 1024 * 1024, 512 * 1024, extracted, used ) );

        // damaged text
        MAIL::CEMailOutgoing outgoing = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_BODY_TEXT, &data );
        COM::CEMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( outgoing, message, reason ) );
        COM::CEMailMessage damaged = message;
        const Int32 begin = damaged.textBody.HasSubstr( "\n\n" );
        ASSERT_TRUE( begin > 0 );
        char* text = const_cast< char* >( damaged.textBody.C_String() );
        // flips a character in the middle of the encoded block
        const Int32 mid = damaged.textBody.HasSubstr( "-----END" ) - 120;
        ASSERT_TRUE( mid > begin );
        text[ mid ] = ( 'A' == text[ mid ] ) ? 'B' : 'A';
        ASSERT_TRUE( MAIL::MAILPAYLOAD_DAMAGED == MAIL::CEMailMatchCodec::ExtractData( damaged, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 1024 * 1024, 512 * 1024, extracted, used ) );

        // the cap of the text
        ASSERT_TRUE( MAIL::MAILPAYLOAD_TOO_LARGE == MAIL::CEMailMatchCodec::ExtractData( message, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 1024 * 1024, 1000, extracted, used ) );

        // the cap of an attachment, whatever its name
        COM::CEMailMessage big;
        CORE::CDynamicBuffer bigData = MakeBytes( 5000, 14 );
        big.attachments.push_back( COM::CEMailAttachment( "../../../etc/passwd", "application/octet-stream", bigData.GetConstBufferPtr(), bigData.GetDataSize() ) );
        ASSERT_TRUE( MAIL::MAILPAYLOAD_TOO_LARGE == MAIL::CEMailMatchCodec::ExtractData( big, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 4096, 512 * 1024, extracted, used ) );
        ASSERT_TRUE( MAIL::MAILPAYLOAD_OK == MAIL::CEMailMatchCodec::ExtractData( big, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 8192, 512 * 1024, extracted, used ) && SameBytes( bigData, extracted ) );
        // an empty attachment is no data
        COM::CEMailMessage empty;
        empty.attachments.push_back( COM::CEMailAttachment( "x.rdxturn", "application/octet-stream", "", 0 ) );
        ASSERT_TRUE( MAIL::MAILPAYLOAD_NONE == MAIL::CEMailMatchCodec::ExtractData( empty, id, 0, MAIL::CEMailMatchCodec::ArmorLabelTurn, 8192, 512 * 1024, extracted, used ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: nothing in a title, a name or a note can add a header or a recipient to the mail" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: nothing in a title, a name or a note can add a header or a recipient to the mail" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        CORE::CDynamicBuffer data = MakeBytes( 100, 15 );
        MAIL::CEMailOutgoing outgoing = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_ATTACHMENT, &data );
        outgoing.title = "Game\r\nBcc: evil@mail.test";
        outgoing.actorName = "Alice\r\nX-Evil: yes";
        outgoing.nextName = "Bob\r\n\r\nInjected body";
        outgoing.note = "note\r\nBcc: evil@mail.test";
        COM::CEMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( outgoing, message, reason ) );
        COM::CEMailMessage received;
        CORE::CString wire;
        ASSERT_TRUE( ThroughTheWire( message, received, &wire ) );
        ASSERT_FALSE( received.headers.Has( "Bcc" ) || received.headers.Has( "X-Evil" ) );
        ASSERT_TRUE( 1 == received.to.size() && 1 == received.cc.size() && received.bcc.empty() );
        ASSERT_TRUE( wire.HasSubstr( "\r\nBcc: evil" ) < 0 && wire.HasSubstr( "\r\nX-Evil" ) < 0 );

        // an address that is not an address cannot be mailed
        MAIL::CEMailOutgoing bad = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_ATTACHMENT, &data );
        bad.to.clear();
        bad.to.push_back( COM::CEMailAddress( "Bob", "bob@mail.test\r\nBcc: evil@mail.test" ) );
        COM::CEMailMessage refused;
        ASSERT_FALSE( MAIL::CEMailMatchCodec::Compose( bad, refused, reason ) );
        MAIL::CEMailOutgoing noRecipient = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_ATTACHMENT, &data );
        noRecipient.to.clear();
        noRecipient.cc.clear();
        ASSERT_FALSE( MAIL::CEMailMatchCodec::Compose( noRecipient, refused, reason ) );
        MAIL::CEMailOutgoing noMatch = MakeOutgoing( "not-a-match-id", MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_ATTACHMENT, &data );
        ASSERT_FALSE( MAIL::CEMailMatchCodec::Compose( noMatch, refused, reason ) );
        // a mail that carries data needs it
        MAIL::CEMailOutgoing noData = MakeOutgoing( id, MAIL::MAILKIND_TURN, MAIL::MAILTRANSFER_ATTACHMENT, GUCEF_NULL );
        ASSERT_FALSE( MAIL::CEMailMatchCodec::Compose( noData, refused, reason ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the small mails (ACK, RESEND) carry no data, and the readable text asks nothing of the reader" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: the small mails (ACK, RESEND) carry no data, and the readable text asks nothing of the reader" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        MAIL::CEMailOutgoing ack = MakeOutgoing( id, MAIL::MAILKIND_ACK, MAIL::MAILTRANSFER_ATTACHMENT, GUCEF_NULL );
        ack.cc.clear();
        ack.packageSha256 = "cafe";
        COM::CEMailMessage message;
        CORE::CString reason;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( ack, message, reason ) );
        COM::CEMailMessage received;
        CORE::CString wire;
        ASSERT_TRUE( ThroughTheWire( message, received, &wire ) );
        ASSERT_TRUE( received.attachments.empty() && wire.ByteSize() < 1500 );
        MAIL::CEMailIncoming incoming;
        MAIL::CEMailMatchCodec::ReadHeaders( received.headers, incoming );
        ASSERT_TRUE( incoming.valid && MAIL::MAILKIND_ACK == incoming.kind && incoming.packageSha256 == "cafe" );

        MAIL::CEMailOutgoing resend = MakeOutgoing( id, MAIL::MAILKIND_RESEND, MAIL::MAILTRANSFER_ATTACHMENT, GUCEF_NULL );
        resend.wantTransfer = "BODY_TEXT";
        ASSERT_TRUE( MAIL::CEMailMatchCodec::Compose( resend, message, reason ) );
        ASSERT_TRUE( ThroughTheWire( message, received ) );
        MAIL::CEMailMatchCodec::ReadHeaders( received.headers, incoming );
        ASSERT_TRUE( incoming.valid && MAIL::MAILKIND_RESEND == incoming.kind && incoming.wantTransfer == "BODY_TEXT" && 3 == incoming.revision );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the headers of a mail from outside are read strictly: protocol, match id, kind and revision" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: the headers of a mail from outside are read strictly: protocol, match id, kind and revision" ), "not the case that was asked for" );
    try
    {
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        MAIL::CEMailIncoming incoming;
        COM::CEMailHeaders headers;
        MAIL::CEMailMatchCodec::ReadHeaders( headers, incoming );
        ASSERT_FALSE( incoming.valid );

        headers.Add( "X-Redux-Match", id );
        headers.Add( "X-Redux-Kind", "TURN" );
        headers.Add( "X-Redux-Revision", "7" );
        headers.Add( "X-Redux-Protocol", "1" );
        headers.Add( "Message-ID", "<abc@mail.test>" );
        headers.Add( "From", "Alice <Alice@Mail.Test>" );
        MAIL::CEMailMatchCodec::ReadHeaders( headers, incoming );
        ASSERT_TRUE( incoming.valid && 7 == incoming.revision && incoming.messageId == "<abc@mail.test>" && incoming.fromAddress.Lowercase() == "alice@mail.test" );

        const char* badValues[][2] = { { "X-Redux-Protocol", "2" }, { "X-Redux-Protocol", "x" }, { "X-Redux-Protocol", "" }, { "X-Redux-Match", "3f2504e0" }, { "X-Redux-Match", "../../x" },
                                       { "X-Redux-Kind", "SHUTDOWN" }, { "X-Redux-Revision", "-1" }, { "X-Redux-Revision", "99999999999999" }, { "X-Redux-Revision", "1e3" } };
        for ( int i=0; i<9; ++i )
        {
            COM::CEMailHeaders changed = headers;
            changed.Set( badValues[ i ][ 0 ], badValues[ i ][ 1 ] );
            MAIL::CEMailIncoming result;
            MAIL::CEMailMatchCodec::ReadHeaders( changed, result );
            ASSERT_FALSE( result.valid );
        }
        // the match id is read in the one form it is used in
        COM::CEMailHeaders upper = headers;
        upper.Set( "X-Redux-Match", id.Uppercase() );
        MAIL::CEMailMatchCodec::ReadHeaders( upper, incoming );
        ASSERT_TRUE( !incoming.valid || incoming.matchId == id );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: bounces, automatic replies and bulk mail are recognized and the mail of a person is not" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: bounces, automatic replies and bulk mail are recognized and the mail of a person is not" ), "not the case that was asked for" );
    try
    {
        ASSERT_FALSE( IsAutomaticWith( GUCEF_NULL, "" ) );
        ASSERT_TRUE( IsAutomaticWith( "Auto-Submitted", "auto-replied" ) );
        ASSERT_TRUE( IsAutomaticWith( "Auto-Submitted", "auto-generated" ) );
        ASSERT_FALSE( IsAutomaticWith( "Auto-Submitted", "no" ) );
        ASSERT_TRUE( IsAutomaticWith( "Precedence", "bulk" ) );
        ASSERT_TRUE( IsAutomaticWith( "Precedence", "junk" ) );
        ASSERT_TRUE( IsAutomaticWith( "X-Autoreply", "yes" ) );
        ASSERT_TRUE( IsAutomaticWith( "X-Autorespond", "yes" ) );
        ASSERT_TRUE( IsAutomaticWith( "X-Failed-Recipients", "bob@mail.test" ) );
        ASSERT_TRUE( IsAutomaticWith( "Return-Path", "<>" ) );
        ASSERT_TRUE( IsAutomaticWith( GUCEF_NULL, "", "MAILER-DAEMON@mail.test" ) );
        ASSERT_TRUE( IsAutomaticWith( GUCEF_NULL, "", "postmaster@mail.test" ) );
        ASSERT_TRUE( IsAutomaticWith( GUCEF_NULL, "", "bob@mail.test", "multipart/report; report-type=delivery-status; boundary=zz" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: a signed record goes to bytes and back and is checked for size and shape" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: a signed record goes to bytes and back and is checked for size and shape" ), "not the case that was asked for" );
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
        MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, "record", false );
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER, "BODY_TEXT" );
        MULTIPLAY::CMatchRecord record( info );
        ASSERT_TRUE( record.Sign( ids[ 0 ] ) );

        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SerializeToBytes( record, bytes, 256 * 1024 ) );
        ASSERT_TRUE( bytes.GetDataSize() > 100 && bytes.GetDataSize() < 32 * 1024 );
        MULTIPLAY::CMatchRecord read;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), read, 256 * 1024 ) );
        CORE::CString why;
        ASSERT_TRUE( read.Verify( why ) && read.info.GetDigest() == record.info.GetDigest() && "BODY_TEXT" == read.info.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ) );

        // the cap holds on both sides
        CORE::CDynamicBuffer tooSmall;
        ASSERT_FALSE( MAIL::CEMailMatchCodec::SerializeToBytes( record, tooSmall, 100 ) );
        MULTIPLAY::CMatchRecord refused;
        ASSERT_FALSE( MAIL::CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), refused, bytes.GetDataSize() - 1 ) );
        // text that is no record
        const char* junk[] = { "", "not json", "{", "[1,2,3]", "{\"a\":", "\x01\x02\x03" };
        for ( int i=0; i<6; ++i )
        {
            MULTIPLAY::CMatchRecord target;
            ASSERT_FALSE( MAIL::CEMailMatchCodec::DeserializeFromBytes( junk[ i ], (UInt32) strlen( junk[ i ] ), target, 256 * 1024 ) && target.Verify( why ) );
        }
        // nesting a thousand deep is not a record and not a crash
        CORE::CString deep;
        for ( int i=0; i<1000; ++i ) deep += "{\"a\":";
        deep += "1";
        for ( int i=0; i<1000; ++i ) deep += "}";
        MULTIPLAY::CMatchRecord deepTarget;
        ASSERT_FALSE( MAIL::CEMailMatchCodec::DeserializeFromBytes( deep.C_String(), deep.ByteSize() - 1, deepTarget, 256 * 1024 ) && deepTarget.Verify( why ) );
        // a record that was changed no longer verifies
        CORE::CString text( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
        const Int32 at = text.HasSubstr( "record" );
        ASSERT_TRUE( at >= 0 );
        char* raw = const_cast< char* >( text.C_String() );
        raw[ at ] = 'R';
        MULTIPLAY::CMatchRecord changed;
        if ( MAIL::CEMailMatchCodec::DeserializeFromBytes( text.C_String(), text.ByteSize() - 1, changed, 256 * 1024 ) )
            ASSERT_FALSE( changed.Verify( why ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: nudges and leaves are signed by a participant for one match and one kind, and nothing else passes" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Codec: nudges and leaves are signed by a participant for one match and one kind, and nothing else passes" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
        for ( int i=0; i<4; ++i )
        {
            MULTIPLAY::CPlayerIdentity identity;
            ASSERT_TRUE( TESTFW::MakeTestIdentity( i == 0 ? "Alice" : ( i == 1 ? "Bob" : ( i == 2 ? "Carol" : "Mallory" ) ), identity ) );
            ids.push_back( identity );
        }
        GUCEF::vector< MULTIPLAY::CPlayerIdentity > three( ids.begin(), ids.begin() + 3 );
        MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( three, "statement", true );
        MULTIPLAY::CMatchInfo other = TESTFW::MakeTestMatchInfo( three, "other", true );

        MAIL::CEMailSignedStatement nudge;
        nudge.matchId = info.matchId;
        nudge.statementKind = MAIL::CEMailSignedStatement::KindNudge;
        nudge.targetUserId = ids[ 1 ].userId;
        nudge.revision = 4;
        nudge.createdAt = 1700000000000LL;
        ASSERT_TRUE( nudge.Sign( ids[ 0 ] ) && nudge.userId == ids[ 0 ].userId && 128 == nudge.signature.Length() );
        CORE::CString why;
        ASSERT_TRUE( nudge.Verify( info, why ) );

        // another match, another kind, a changed field, another signer
        ASSERT_FALSE( nudge.Verify( other, why ) );
        MAIL::CEMailSignedStatement asResign = nudge;
        asResign.statementKind = MAIL::CEMailSignedStatement::KindResign;
        ASSERT_FALSE( asResign.Verify( info, why ) );
        MAIL::CEMailSignedStatement changedTarget = nudge;
        changedTarget.targetUserId = ids[ 2 ].userId;
        ASSERT_FALSE( changedTarget.Verify( info, why ) );
        MAIL::CEMailSignedStatement changedRevision = nudge;
        changedRevision.revision = 5;
        ASSERT_FALSE( changedRevision.Verify( info, why ) );
        MAIL::CEMailSignedStatement claimsBob = nudge;
        claimsBob.userId = ids[ 1 ].userId;
        ASSERT_FALSE( claimsBob.Verify( info, why ) );
        // someone who is not a participant, with a good signature of their own
        MAIL::CEMailSignedStatement outsider;
        outsider.matchId = info.matchId;
        outsider.statementKind = MAIL::CEMailSignedStatement::KindNudge;
        outsider.targetUserId = ids[ 1 ].userId;
        ASSERT_TRUE( outsider.Sign( ids[ 3 ] ) );
        ASSERT_FALSE( outsider.Verify( info, why ) );
        // an unknown kind cannot be signed
        MAIL::CEMailSignedStatement unknownKind;
        unknownKind.matchId = info.matchId;
        unknownKind.statementKind = "wipe";
        ASSERT_FALSE( unknownKind.Sign( ids[ 0 ] ) );

        MAIL::CEMailSignedStatement resign;
        resign.matchId = info.matchId;
        resign.statementKind = MAIL::CEMailSignedStatement::KindResign;
        ASSERT_TRUE( resign.Sign( ids[ 2 ] ) && resign.Verify( info, why ) );

        // as bytes, and back
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::SerializeToBytes( nudge, bytes, 64 * 1024 ) && bytes.GetDataSize() < 2048 );
        MAIL::CEMailSignedStatement read;
        ASSERT_TRUE( MAIL::CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), read, 64 * 1024 ) && read.Verify( info, why ) );
        ASSERT_TRUE( read.userId == nudge.userId && read.targetUserId == nudge.targetUserId && 4 == read.revision && read.createdAt == nudge.createdAt );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
