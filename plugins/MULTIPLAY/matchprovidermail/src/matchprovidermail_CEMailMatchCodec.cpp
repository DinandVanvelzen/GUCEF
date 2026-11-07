/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#include "matchprovidermail_CEMailMatchCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CEMailMatchCodec::HeaderMatch = "X-Redux-Match";
const char* CEMailMatchCodec::HeaderRevision = "X-Redux-Revision";
const char* CEMailMatchCodec::HeaderKind = "X-Redux-Kind";
const char* CEMailMatchCodec::HeaderProtocol = "X-Redux-Protocol";
const char* CEMailMatchCodec::HeaderPackage = "X-Redux-Package";
const char* CEMailMatchCodec::HeaderTransfer = "X-Redux-Transfer";
const char* CEMailMatchCodec::HeaderRematchOf = "X-Redux-Rematch-Of";
const char* CEMailMatchCodec::ArmorLabelTurn = "MATCH TURN";
const char* CEMailMatchCodec::ArmorLabelData = "MATCH DATA";
const char* CEMailSignedStatement::KindNudge = "nudge";
const char* CEMailSignedStatement::KindResign = "resign";

static const char* const g_kindNames[] = { "INVITE", "ACCEPT", "DECLINE", "TURN", "ACK", "NUDGE", "RESIGN", "FINISH", "RESEND", "PROPOSAL", "VOTE", "INTRO_REQUEST", "INTRO_REPLY", "CANCEL", "INCOMPATIBLE", "ROSTER", "DRAFT_PROPOSAL", "DRAFT_VOTE" };
static const UInt32 g_kindCount = sizeof( g_kindNames ) / sizeof( g_kindNames[ 0 ] );
static const UInt32 g_maxNestingDepth = 16;
static const UInt32 g_maxRevisionDigits = 9;
static const UInt32 g_maxBodyTextBytes = 8 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Text for one line of the readable part: control characters become blanks, blanks are not repeated; the limit is in bytes */
static CORE::CString
OneLine( const CORE::CString& text, UInt32 maxBytes )
{GUCEF_TRACE;

    GUCEF::vector< char > out;
    const UInt32 size = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    const char* p = text.C_String();
    bool lastBlank = true;
    for ( UInt32 i=0; i<size && out.size() < maxBytes; ++i )
    {
        const unsigned char c = (unsigned char) p[ i ];
        if ( c < 0x20 || 0x7F == c || ' ' == c )
        {
            if ( !lastBlank )
                out.push_back( ' ' );
            lastBlank = true;
        }
        else
        {
            out.push_back( (char) c );
            lastBlank = false;
        }
    }
    while ( !out.empty() && ' ' == out.back() )
        out.pop_back();
    return out.empty() ? CORE::CString() : CORE::CString( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchCodec::SanitizeTitle( const CORE::CString& title )
{GUCEF_TRACE;

    const CORE::CString clean = OneLine( title, MaxSubjectTitleBytes * 2 );
    if ( clean.IsNULLOrEmpty() || clean.ByteSize() - 1 <= MaxSubjectTitleBytes )
        return clean;
    // cut on a character boundary: not in front of a continuation byte
    const char* p = clean.C_String();
    UInt32 cut = MaxSubjectTitleBytes;
    while ( cut > 0 && 0x80 == ( (unsigned char) p[ cut ] & 0xC0 ) )
        --cut;
    while ( cut > 0 && ' ' == p[ cut - 1 ] )
        --cut;
    return 0 == cut ? CORE::CString() : CORE::CString( p, cut );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchCodec::MakeSubject( const CORE::CString& matchId, UInt32 revision, UInt32 turn, const CORE::CString& title )
{GUCEF_TRACE;

    CORE::CString subject = "[Redux " + MULTIPLAY::CMatchId::Short( matchId ) + " r" + CORE::UInt32ToString( revision ) + " t" + CORE::UInt32ToString( turn ) + "]";
    const CORE::CString clean = SanitizeTitle( title );
    if ( !clean.IsNULLOrEmpty() )
        subject += " " + clean;
    return subject;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchCodec::MakeAttachmentName( const CORE::CString& matchId, const CORE::CString& part )
{GUCEF_TRACE;

    if ( !MULTIPLAY::CMatchId::IsValid( matchId ) || part.IsNULLOrEmpty() || part.Length() > 16 )
        return CORE::CString::Empty;
    bool allDigitsAfterR = part.Length() > 1 && 'r' == part[ 0 ];
    for ( UInt32 i=0; i<part.Length(); ++i )
    {
        const char c = part[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) ) )
            return CORE::CString::Empty;
        if ( i > 0 && !( c >= '0' && c <= '9' ) )
            allDigitsAfterR = false;
    }
    return "redux-" + MULTIPLAY::CMatchId::Short( matchId ) + "-" + part + ( allDigitsAfterR ? ".rdxturn" : ".rdxdata" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchCodec::MakeMessageId( const CORE::CString& fromAddress )
{GUCEF_TRACE;

    UInt8 random[ 12 ];
    CORE::CRandom::GetRandomBytes( random, sizeof( random ) );
    CORE::CString hex;
    static const char digits[] = "0123456789abcdef";
    for ( UInt32 i=0; i<sizeof( random ); ++i )
    {
        hex += digits[ random[ i ] >> 4 ];
        hex += digits[ random[ i ] & 0x0F ];
    }
    CORE::CString domain = "redux.invalid";
    const Int32 at = fromAddress.HasChar( '@', false );
    if ( at >= 0 && (UInt32) at + 1 < fromAddress.Length() )
    {
        const CORE::CString candidate = fromAddress.SubstrFromRange( (UInt32) at + 1, fromAddress.Length() );
        bool ok = !candidate.IsNULLOrEmpty() && candidate.Length() <= 100;
        for ( UInt32 i=0; ok && i<candidate.Length(); ++i )
        {
            const char c = candidate[ i ];
            ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '-' == c;
        }
        if ( ok )
            domain = candidate;
    }
    return "<" + hex + "@" + domain + ">";
}

/*-------------------------------------------------------------------------*/

static bool
IsHex( const CORE::CString& text, UInt32 maxLength )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() || text.Length() > maxLength )
        return false;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
DescribeKind( const CEMailOutgoing& o, const CORE::CString& title, CORE::CString& headline )
{GUCEF_TRACE;

    const CORE::CString actor = o.actorName.IsNULLOrEmpty() ? CORE::CString( "A player" ) : OneLine( o.actorName, 64 );
    const CORE::CString next = OneLine( o.nextName, 64 );
    switch ( o.kind )
    {
        case MAILKIND_INVITE:   headline = actor + " invites you to the game \"" + title + "\"."; break;
        case MAILKIND_ACCEPT:   headline = actor + " accepted the invitation to the game \"" + title + "\"."; break;
        case MAILKIND_DECLINE:  headline = actor + " declined the invitation to the game \"" + title + "\"."; break;
        case MAILKIND_TURN:     headline = actor + " played turn " + CORE::UInt32ToString( o.turn ) + " of the game \"" + title + "\"." + ( next.IsNULLOrEmpty() ? CORE::CString() : " It is " + next + "'s turn now." ); break;
        case MAILKIND_FINISH:   headline = actor + " played the last turn of the game \"" + title + "\". The game is over."; break;
        case MAILKIND_ACK:      headline = "Turn " + CORE::UInt32ToString( o.revision ) + " of the game \"" + title + "\" arrived."; break;
        case MAILKIND_NUDGE:    headline = actor + " reminds you that it is your turn in the game \"" + title + "\"."; break;
        case MAILKIND_RESIGN:   headline = actor + " left the game \"" + title + "\"."; break;
        case MAILKIND_RESEND:   headline = actor + " asks for turn " + CORE::UInt32ToString( o.revision ) + " of the game \"" + title + "\" to be sent again."; break;
        case MAILKIND_PROPOSAL: headline = actor + " proposes a decision in the game \"" + title + "\"."; break;
        case MAILKIND_VOTE:     headline = actor + " voted on a decision in the game \"" + title + "\"."; break;
        case MAILKIND_INTRO_REQUEST:    headline = actor + " invites you to the game \"" + title + "\" and asks you to introduce yourself."; break;
        case MAILKIND_INTRO_REPLY:      headline = actor + " answered the introduction for the game \"" + title + "\"."; break;
        case MAILKIND_CANCEL:           headline = actor + " cancelled the game \"" + title + "\" before it began."; break;
        case MAILKIND_INCOMPATIBLE:     headline = actor + " cannot play the game \"" + title + "\" and watches instead."; break;
        case MAILKIND_ROSTER:           headline = actor + " published the table of the game \"" + title + "\" as it is now."; break;
        case MAILKIND_DRAFT_PROPOSAL:   headline = actor + " proposes a decision about the table of the game \"" + title + "\"."; break;
        case MAILKIND_DRAFT_VOTE:       headline = actor + " voted on a decision about the table of the game \"" + title + "\"."; break;
        default:                headline = "A message about the game \"" + title + "\"."; break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchCodec::Compose( const CEMailOutgoing& o, COM::CEMailMessage& message, CORE::CString& reason )
{GUCEF_TRACE;

    message = COM::CEMailMessage();
    if ( !MULTIPLAY::CMatchId::IsValid( o.matchId ) )
    {
        reason = "The match id is not valid";
        return false;
    }
    if ( MAILKIND_UNKNOWN == o.kind )
    {
        reason = "The kind of the mail is not known";
        return false;
    }
    const bool needsData = MAILKIND_ACK != o.kind && MAILKIND_RESEND != o.kind;
    if ( needsData && ( GUCEF_NULL == o.data || 0 == o.data->GetDataSize() ) )
    {
        reason = "The mail needs its data";
        return false;
    }
    if ( GUCEF_NULL != o.data && ( o.data->GetDataSize() > MULTIPLAY::CMatchContainer::MaxContainerSize || o.dataRevisionName.IsNULLOrEmpty() ) )
    {
        reason = "The data is too large or has no name";
        return false;
    }

    const CORE::CString title = SanitizeTitle( o.title );
    message.from.push_back( o.from );
    message.to = o.to;
    message.cc = o.cc;
    message.subject = MakeSubject( o.matchId, o.revision, o.turn, title );
    message.inReplyTo.clear();
    if ( !o.inReplyTo.IsNULLOrEmpty() )
        message.inReplyTo.push_back( o.inReplyTo );
    message.references = o.references;

    message.extraHeaders.Add( HeaderMatch, o.matchId );
    message.extraHeaders.Add( HeaderRevision, CORE::UInt32ToString( o.revision ) );
    message.extraHeaders.Add( HeaderKind, MailMatchKindToString( o.kind ) );
    message.extraHeaders.Add( HeaderProtocol, CORE::UInt32ToString( ProtocolVersion ) );
    if ( IsHex( o.packageSha256, 64 ) )
        message.extraHeaders.Add( HeaderPackage, o.packageSha256 );
    TMailTransfer wanted = MAILTRANSFER_ATTACHMENT;
    if ( !o.wantTransfer.IsNULLOrEmpty() && TryStringToMailTransfer( o.wantTransfer, wanted ) )
        message.extraHeaders.Add( HeaderTransfer, MailTransferToString( wanted ) );
    if ( MULTIPLAY::CMatchId::IsValid( o.rematchOf ) )
        message.extraHeaders.Add( HeaderRematchOf, o.rematchOf );

    CORE::CString headline;
    DescribeKind( o, title.IsNULLOrEmpty() ? CORE::CString( "untitled" ) : title, headline );
    CORE::CString text = headline + "\r\n";
    if ( !o.note.IsNULLOrEmpty() )
        text += OneLine( o.note, 200 ) + "\r\n";
    text += "\r\n";
    if ( !o.linkText.IsNULLOrEmpty() )
    {
        // the link lines are made by the provider from its settings and the match id; control characters are still not let through
        CORE::CString::StringVector lines = o.linkText.ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
        {
            const CORE::CString line = OneLine( lines[ i ], 300 );
            if ( !line.IsNULLOrEmpty() )
                text += line + "\r\n";
        }
        text += "\r\n";
    }
    if ( !o.bodyText.IsNULLOrEmpty() )
    {
        // lines of readable text: each one is made safe and the whole is bounded
        CORE::CString::StringVector lines = o.bodyText.ParseElements( '\n', false );
        UInt32 total = 0;
        for ( size_t i=0; i<lines.size() && total < g_maxBodyTextBytes; ++i )
        {
            const CORE::CString line = OneLine( lines[ i ], 300 );
            text += line + "\r\n";
            total += line.Length() + 2;
        }
        text += "\r\n";
    }
    if ( GUCEF_NULL != o.data )
    {
        if ( MAILTRANSFER_ATTACHMENT == o.transfer )
            text += "The game data is in the attachment. Do not change it; the game checks it.\r\n";
        else
            text += "The game data is the block of text below. Do not change it; the game checks it.\r\n";
    }
    text += "This message was sent by the game for the players of this game.\r\n";

    // the data: an attachment or armored text in the body
    GUCEF::vector< const CORE::CDynamicBuffer* > blocks;
    GUCEF::vector< CORE::CString > labels;
    GUCEF::vector< CORE::CString > names;
    if ( GUCEF_NULL != o.data )
    {
        blocks.push_back( o.data );
        labels.push_back( o.dataLabel );
        names.push_back( o.dataRevisionName );
    }
    if ( GUCEF_NULL != o.extraData && o.extraData->GetDataSize() > 0 )
    {
        blocks.push_back( o.extraData );
        labels.push_back( o.extraLabel );
        names.push_back( o.extraRevisionName );
    }
    for ( size_t b=0; b<blocks.size(); ++b )
    {
        if ( labels[ b ].IsNULLOrEmpty() )
        {
            reason = "The data has no label";
            return false;
        }
        if ( MAILTRANSFER_ATTACHMENT == o.transfer )
        {
            const CORE::CString fileName = MakeAttachmentName( o.matchId, names[ b ] );
            if ( fileName.IsNULLOrEmpty() )
            {
                reason = "The name of the attachment cannot be made";
                return false;
            }
            message.attachments.push_back( COM::CEMailAttachment( fileName, "application/octet-stream", blocks[ b ]->GetConstBufferPtr(), blocks[ b ]->GetDataSize() ) );
        }
        else
        {
            UInt32 blockRevision = 0;
            if ( names[ b ].Length() > 1 && 'r' == names[ b ][ 0 ] )
                blockRevision = (UInt32) atol( names[ b ].C_String() + 1 );
            MULTIPLAY::CMatchTextArmor armor( labels[ b ] );
            CORE::CString armored;
            if ( !armor.Encode( o.matchId, blockRevision, blocks[ b ]->GetConstBufferPtr(), blocks[ b ]->GetDataSize(), armored ) )
            {
                reason = "The data is too large to be sent as text";
                return false;
            }
            text += "\r\n" + armored + "\r\n";
        }
    }
    message.textBody = text;

    if ( !message.Validate( reason ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ParseRevision( const CORE::CString& text, UInt32& revision )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() || text.Length() > g_maxRevisionDigits )
        return false;
    UInt32 value = 0;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        if ( c < '0' || c > '9' )
            return false;
        value = value * 10 + (UInt32)( c - '0' );
    }
    revision = value;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchCodec::ReadHeaders( const COM::CEMailHeaders& headers, CEMailIncoming& incoming )
{GUCEF_TRACE;

    incoming = CEMailIncoming();

    incoming.automatic = IsAutomatic( headers );
    CORE::CString::StringVector ids;
    COM::CMimeParser::ExtractMessageIds( headers.Get( "Message-ID" ), ids, 1 );
    if ( !ids.empty() && ids[ 0 ].Length() <= 998 )
        incoming.messageId = ids[ 0 ];
    COM::CEMailAddress from;
    if ( COM::CEMailAddress::Parse( headers.Get( "From" ), from ) )
        incoming.fromAddress = from.GetAddress().Lowercase();

    const CORE::CString rawMatch = headers.Get( HeaderMatch ).Trim( true ).Trim( false );
    incoming.matchId = MULTIPLAY::CMatchId::Normalize( rawMatch );
    if ( incoming.matchId.IsNULLOrEmpty() )
        return;
    const CORE::CString protocol = headers.Get( HeaderProtocol ).Trim( true ).Trim( false );
    UInt32 version = 0;
    if ( !ParseRevision( protocol, version ) || ProtocolVersion != version )
        return;
    incoming.protocolVersion = version;
    if ( !TryStringToMailMatchKind( headers.Get( HeaderKind ).Trim( true ).Trim( false ), incoming.kind ) )
        return;
    if ( headers.Has( HeaderRevision ) && !ParseRevision( headers.Get( HeaderRevision ).Trim( true ).Trim( false ), incoming.revision ) )
        return;
    const CORE::CString sha = headers.Get( HeaderPackage ).Trim( true ).Trim( false );
    if ( IsHex( sha, 64 ) )
        incoming.packageSha256 = sha.Lowercase();
    TMailTransfer transfer = MAILTRANSFER_ATTACHMENT;
    const CORE::CString want = headers.Get( HeaderTransfer ).Trim( true ).Trim( false );
    if ( TryStringToMailTransfer( want, transfer ) )
        incoming.wantTransfer = MailTransferToString( transfer );
    incoming.valid = true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchCodec::IsAutomatic( const COM::CEMailHeaders& headers )
{GUCEF_TRACE;

    const CORE::CString autoSubmitted = headers.Get( "Auto-Submitted" ).Trim( true ).Trim( false ).Lowercase();
    if ( !autoSubmitted.IsNULLOrEmpty() && !autoSubmitted.StartsWith( "no" ) )
        return true;
    const CORE::CString precedence = headers.Get( "Precedence" ).Trim( true ).Trim( false ).Lowercase();
    if ( "bulk" == precedence || "junk" == precedence || "list" == precedence )
        return true;
    if ( headers.Has( "X-Autoreply" ) || headers.Has( "X-Autorespond" ) || headers.Has( "X-Failed-Recipients" ) )
        return true;
    if ( headers.Has( "Return-Path" ) && headers.Get( "Return-Path" ).Trim( true ).Trim( false ) == "<>" )
        return true;
    if ( headers.Get( "Content-Type" ).Trim( true ).Trim( false ).Lowercase().StartsWith( "multipart/report" ) )
        return true;
    COM::CEMailAddress from;
    if ( COM::CEMailAddress::Parse( headers.Get( "From" ), from ) )
    {
        const CORE::CString local = from.localPart.Lowercase();
        if ( "mailer-daemon" == local || "postmaster" == local || "mail-daemon" == local )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
EndsWithName( const CORE::CString& name, const char* suffix )
{GUCEF_TRACE;

    const CORE::CString lower = name.Lowercase();
    return lower.EndsWith( suffix );
}

/*-------------------------------------------------------------------------*/

TMailPayloadResult
CEMailMatchCodec::ExtractData( const COM::CEMailMessage& message  ,
                              const CORE::CString& matchId      ,
                              UInt32 wantedRevision             ,
                              const CORE::CString& label        ,
                              UInt32 maxBytes                   ,
                              UInt32 maxTextBytes               ,
                              CORE::CDynamicBuffer& data        ,
                              TMailTransfer& usedTransfer       )
{GUCEF_TRACE;

    data.Clear();

    // an attachment, whatever it is called: the name is only a hint to tell the two kinds of data apart
    const bool wantsTurn = label == ArmorLabelTurn;
    const char* wantedSuffix = wantsTurn ? ".rdxturn" : ".rdxdata";
    const char* otherSuffix = wantsTurn ? ".rdxdata" : ".rdxturn";
    const COM::CEMailAttachment* chosen = GUCEF_NULL;
    const COM::CEMailAttachment* firstUnnamed = GUCEF_NULL;
    for ( size_t i=0; i<message.attachments.size(); ++i )
    {
        const COM::CEMailAttachment& attachment = message.attachments[ i ];
        if ( 0 == attachment.data.GetDataSize() )
            continue;
        if ( EndsWithName( attachment.fileName, wantedSuffix ) )
        {
            chosen = &attachment;
            break;
        }
        if ( GUCEF_NULL == firstUnnamed && !EndsWithName( attachment.fileName, otherSuffix ) )
            firstUnnamed = &attachment;
    }
    if ( GUCEF_NULL == chosen )
        chosen = firstUnnamed;
    if ( GUCEF_NULL != chosen )
    {
        if ( chosen->data.GetDataSize() > maxBytes )
            return MAILPAYLOAD_TOO_LARGE;
        data.Append( chosen->data.GetConstBufferPtr(), chosen->data.GetDataSize() );
        usedTransfer = MAILTRANSFER_ATTACHMENT;
        return MAILPAYLOAD_OK;
    }

    // armored text
    const CORE::CString& text = message.textBody.IsNULLOrEmpty() ? message.htmlBody : message.textBody;
    if ( text.IsNULLOrEmpty() )
        return MAILPAYLOAD_NONE;
    const CORE::CString beginMarker = "-----BEGIN " + label + "-----";
    const Int32 begin = text.HasSubstr( beginMarker );
    if ( begin < 0 )
        return MAILPAYLOAD_NONE;
    const Int32 end = text.HasSubstr( "-----END " + label + "-----", begin );
    const UInt32 blockSize = end < 0 ? text.Length() - (UInt32) begin : (UInt32) end - (UInt32) begin;
    if ( blockSize > maxTextBytes )
        return MAILPAYLOAD_TOO_LARGE;
    MULTIPLAY::CMatchTextArmor armor( label );
    UInt32 found = 0;
    const MULTIPLAY::TMatchArmorResult result = armor.Decode( text.C_String(), text.Length(), matchId, wantedRevision, data, &found );
    switch ( result )
    {
        case MULTIPLAY::MATCHARMOR_OK:
        {
            if ( data.GetDataSize() > maxBytes )
            {
                data.Clear();
                return MAILPAYLOAD_TOO_LARGE;
            }
            usedTransfer = MAILTRANSFER_BODY_TEXT;
            return MAILPAYLOAD_OK;
        }
        case MULTIPLAY::MATCHARMOR_NO_BLOCK:        return MAILPAYLOAD_NONE;
        case MULTIPLAY::MATCHARMOR_TOO_LARGE:       return MAILPAYLOAD_TOO_LARGE;
        case MULTIPLAY::MATCHARMOR_BAD_ENCODING:    return MAILPAYLOAD_BAD_ENCODING;
        default:                                    return MAILPAYLOAD_DAMAGED;
    }
}

/*-------------------------------------------------------------------------*/

static bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchCodec::SerializeToBytes( const CORE::CIDataNodeSerializable& source, CORE::CDynamicBuffer& bytes, UInt32 maxBytes )
{GUCEF_TRACE;

    bytes.Clear();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    CORE::CDataNode root( "MatchData" );
    CORE::CDataNodeSerializableSettings settings;
    if ( !source.Serialize( root, settings ) )
        return false;
    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( &buffer, false );
    if ( !codec->StoreDataTree( &root, &access ) || 0 == buffer.GetDataSize() || buffer.GetDataSize() > maxBytes )
        return false;
    bytes.Append( buffer.GetConstBufferPtr(), buffer.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The deepest nesting of brackets in JSON text: a bomb of nested lists is refused before a parser sees it */
static bool
IsNestingWithinLimit( const char* text, UInt32 size, UInt32 limit )
{GUCEF_TRACE;

    UInt32 depth = 0;
    bool inString = false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = text[ i ];
        if ( inString )
        {
            if ( '\\' == c )
                ++i;
            else
            if ( '"' == c )
                inString = false;
            continue;
        }
        if ( '"' == c )
            inString = true;
        else
        if ( '{' == c || '[' == c )
        {
            if ( ++depth > limit )
                return false;
        }
        else
        if ( ( '}' == c || ']' == c ) && depth > 0 )
            --depth;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchCodec::DeserializeFromBytes( const void* data, UInt32 size, CORE::CIDataNodeSerializable& target, UInt32 maxBytes )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || 0 == size || size > maxBytes )
        return false;
    if ( !IsNestingWithinLimit( static_cast< const char* >( data ), size, g_maxNestingDepth ) )
        return false;
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( data, size );
    CORE::CDynamicBufferAccess access( buffer );
    CORE::CDataNode root;
    if ( !codec->BuildDataTree( &root, &access ) )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    return target.Deserialize( root, settings );
}

/*-------------------------------------------------------------------------*/

TMailTransfer
CEMailMatchCodec::GetTransfer( const MULTIPLAY::CMatchInfo& info )
{GUCEF_TRACE;

    TMailTransfer transfer = MAILTRANSFER_ATTACHMENT;
    if ( !TryStringToMailTransfer( info.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ), transfer ) )
        return MAILTRANSFER_ATTACHMENT;
    return transfer;
}

/*-------------------------------------------------------------------------*/

TMailDistribution
CEMailMatchCodec::GetDistribution( const MULTIPLAY::CMatchInfo& info )
{GUCEF_TRACE;

    TMailDistribution distribution = MAILDISTRIBUTION_ALL;
    if ( !TryStringToMailDistribution( info.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION ), distribution ) )
        return MAILDISTRIBUTION_ALL;
    return distribution;
}

/*-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
