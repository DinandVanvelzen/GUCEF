/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_CMAILMESSAGE_H
#include "gucefCOM_CMailMessage.h"
#define GUCEF_COM_CMAILMESSAGE_H
#endif /* GUCEF_COM_CMAILMESSAGE_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a mail of a match is, the value of the header X-Redux-Kind */
enum EMailMatchKind
{
    MAILKIND_INVITE   = 0 ,     /**< the signed match record to an invitee, with the first turn when there is one */
    MAILKIND_ACCEPT   = 1 ,     /**< a signed acceptance of the record */
    MAILKIND_DECLINE  = 2 ,
    MAILKIND_TURN     = 3 ,     /**< a turn: the container with the signed preview and the package */
    MAILKIND_ACK      = 4 ,     /**< a turn arrived */
    MAILKIND_NUDGE    = 5 ,     /**< a signed reminder to the participant whose turn it is */
    MAILKIND_RESIGN   = 6 ,     /**< a signed leave */
    MAILKIND_FINISH   = 7 ,     /**< the last turn of a match */
    MAILKIND_RESEND   = 8 ,     /**< a request to send the turn of a revision again */
    MAILKIND_PROPOSAL = 9 ,     /**< a signed proposal of a decision */
    MAILKIND_VOTE     = 10 ,    /**< a signed vote on a proposal */
    MAILKIND_INTRO_REQUEST = 11 ,   /**< the creator of a game asks an address to introduce itself: the signed request (design 18) */
    MAILKIND_INTRO_REPLY   = 12 ,   /**< the answer: a signed introduction or an unsigned decline */
    MAILKIND_CANCEL   = 13 ,    /**< the creator (or a boss) closes a match that has not started: a signed decision, the proposal of the kind CANCEL */
    MAILKIND_INCOMPATIBLE = 14 ,/**< a participant says, signed, that it watches instead of playing, and why (the first turn cannot be played here, or by choice): a signed proposal of the kind MOVE_TO_SPECTATOR */
    MAILKIND_ROSTER   = 15 ,    /**< the host of a game that has no record yet publishes the signed table of the draft (a CMatchRoster) to every player */
    MAILKIND_DRAFT_PROPOSAL = 16 ,  /**< a signed proposal of the draft stage (force start, kick, the own spectating), to the host and to every other player */
    MAILKIND_DRAFT_VOTE     = 17 ,  /**< a signed vote on such a proposal, to the host and to every other player */
    MAILKIND_UNKNOWN  = 18
};
typedef enum EMailMatchKind TMailMatchKind;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailMatchKindToString( TMailMatchKind kind );

/** @return false (kind is MAILKIND_UNKNOWN) when the text is no kind */
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailMatchKind( const CORE::CString& text, TMailMatchKind& kind );

/** How the data of a mail travels, the match setting "mailTransfer" */
enum EMailTransfer
{
    MAILTRANSFER_ATTACHMENT = 0 ,
    MAILTRANSFER_BODY_TEXT  = 1
};
typedef enum EMailTransfer TMailTransfer;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailTransferToString( TMailTransfer transfer );
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailTransfer( const CORE::CString& text, TMailTransfer& transfer );

/** To whom a turn is mailed, the match setting "mailDistribution" */
enum EMailDistribution
{
    MAILDISTRIBUTION_NEXT_ONLY = 0 ,
    MAILDISTRIBUTION_ALL       = 1
};
typedef enum EMailDistribution TMailDistribution;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailDistributionToString( TMailDistribution distribution );
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailDistribution( const CORE::CString& text, TMailDistribution& distribution );

/** What was found when the data was taken out of a mail */
enum EMailPayloadResult
{
    MAILPAYLOAD_OK           = 0 ,
    MAILPAYLOAD_NONE         = 1 ,  /**< no attachment and no block of text for the match: the carrier may have removed it */
    MAILPAYLOAD_TOO_LARGE    = 2 ,
    MAILPAYLOAD_DAMAGED      = 3 ,  /**< the text was damaged on its way: ask for a new copy */
    MAILPAYLOAD_BAD_ENCODING = 4
};
typedef enum EMailPayloadResult TMailPayloadResult;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailPayloadResultToString( TMailPayloadResult result );

/** The setting keys of a match record that the mail provider reads */
#define GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER         "mailTransfer"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION     "mailDistribution"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a mail about a match says in its headers: all that is needed to decide whether to read the rest */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIncoming
{
    public:

    bool valid;                     /**< the headers of a mail of this protocol */
    TMailMatchKind kind;
    CORE::CString matchId;
    UInt32 revision;
    UInt32 protocolVersion;
    CORE::CString messageId;        /**< with angle brackets, empty when the mail has none */
    CORE::CString fromAddress;      /**< the address of the first From, as written, empty when there is none */
    CORE::CString packageSha256;    /**< X-Redux-Package, empty when absent */
    CORE::CString wantTransfer;     /**< X-Redux-Transfer of a resend request: the form the answer should have, empty for the match's own */
    bool automatic;                 /**< a bounce, an automatic reply or a bulk message: never answered, never read */

    CMailIncoming( void );
};

/** A mail to be written: the data of the match, and the people it is for */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailOutgoing
{
    public:

    TMailMatchKind kind;
    CORE::CString matchId;
    UInt32 revision;
    UInt32 turn;
    CORE::CString title;                    /**< as the players know the match; made safe for the subject */
    COM::CMailAddress from;
    COM::CMailAddress::TMailAddressVector to;
    COM::CMailAddress::TMailAddressVector cc;
    CORE::CString inReplyTo;                /**< the message id of the previous mail of the match, with brackets */
    CORE::CString::StringVector references;
    CORE::CString actorName;                /**< who played, accepted, ...: for the readable text */
    CORE::CString nextName;                 /**< whose turn it is now */
    CORE::CString note;                     /**< an extra sentence for the readable text */
    CORE::CString bodyText;                 /**< lines of readable text for the body (the introduction mails), line ends CR LF or LF; every line is made safe and the whole is cut at a few KB */
    CORE::CString packageSha256;
    CORE::CString wantTransfer;
    CORE::CString rematchOf;                /**< the id of the match this one is the rematch of */
    CORE::CString linkText;                 /**< lines with the links that open the match in the application */
    TMailTransfer transfer;
    const CORE::CDynamicBuffer* data;       /**< the bytes that travel (a container, or a signed statement as data tree text), GUCEF_NULL for none */
    CORE::CString dataLabel;                /**< the label in the marker lines when the data travels as text: "MATCH TURN" or "MATCH DATA" */
    CORE::CString dataRevisionName;         /**< the part of the attachment name after the match: "r3" or "record" */
    const CORE::CDynamicBuffer* extraData;  /**< a second block that travels with the first (the first turn with an invitation), GUCEF_NULL for none */
    CORE::CString extraLabel;
    CORE::CString extraRevisionName;

    CMailOutgoing( void );
};

/**
 *  Writes and reads the mails of the mail match provider; no network and no state. A mail of a match has the headers
 *  X-Redux-Match, X-Redux-Revision, X-Redux-Kind and X-Redux-Protocol (1), the subject
 *  "[Redux <first 8 of the match id> r<revision> t<turn>] <title>", a plain text body that a person can read without
 *  the game, and the data either as the attachment redux-<id8>-<name>.rdxturn or as armored text in the body between
 *  marker lines. Everything read from a mail is hostile: sizes are checked before anything is copied, attachment names are
 *  never used.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailMatchCodec
{
    public:

    static const char* HeaderMatch;             /**< "X-Redux-Match" */
    static const char* HeaderRevision;
    static const char* HeaderKind;
    static const char* HeaderProtocol;
    static const char* HeaderPackage;
    static const char* HeaderTransfer;
    static const char* HeaderRematchOf;
    static const char* ArmorLabelTurn;          /**< "MATCH TURN" */
    static const char* ArmorLabelData;          /**< "MATCH DATA" */
    static const UInt32 ProtocolVersion = 1;
    static const UInt32 MaxSubjectTitleBytes = 80;

    /** The title without anything that could end a header or confuse a list: control characters become spaces, at most MaxSubjectTitleBytes bytes on a character boundary */
    static CORE::CString SanitizeTitle( const CORE::CString& title );

    /** "[Redux 3f2504e0 r3 t3] title" */
    static CORE::CString MakeSubject( const CORE::CString& matchId, UInt32 revision, UInt32 turn, const CORE::CString& title );

    /** "redux-3f2504e0-r3.rdxturn": a name made here, from the match id only */
    static CORE::CString MakeAttachmentName( const CORE::CString& matchId, const CORE::CString& part );

    /** A new Message-ID with angle brackets, for a mail from the address: unique and unguessable */
    static CORE::CString MakeMessageId( const CORE::CString& fromAddress );

    /** Composes the message; false with a reason (no secrets, no addresses) when it is not valid */
    static bool Compose( const CMailOutgoing& outgoing, COM::CMailMessage& message, CORE::CString& reason );

    /** Reads what the headers say. Never fails: 'valid' says whether this is a mail of a match */
    static void ReadHeaders( const COM::CMailHeaders& headers, CMailIncoming& incoming );

    /** Whether the headers are those of a bounce, an automatic reply or a bulk message */
    static bool IsAutomatic( const COM::CMailHeaders& headers );

    /**
     *  Takes the data out of a mail: the attachment (any name) when there is one, else the block of text of the match in the body.
     *
     *  @param label the label of the marker lines
     *  @param wantedRevision the revision of the block of text, 0 for any
     *  @param maxBytes the cap of the data
     */
    static TMailPayloadResult ExtractData( const COM::CMailMessage& message  ,
                                           const CORE::CString& matchId      ,
                                           UInt32 wantedRevision             ,
                                           const CORE::CString& label        ,
                                           UInt32 maxBytes                   ,
                                           UInt32 maxTextBytes               ,
                                           CORE::CDynamicBuffer& data        ,
                                           TMailTransfer& usedTransfer       );

    /** The text of a signed record as bytes (a data tree stored as JSON), within maxBytes */
    static bool SerializeToBytes( const CORE::CIDataNodeSerializable& source, CORE::CDynamicBuffer& bytes, UInt32 maxBytes );

    /** The reverse, for bytes from outside: bounded, and the target decides what it accepts */
    static bool DeserializeFromBytes( const void* data, UInt32 size, CORE::CIDataNodeSerializable& target, UInt32 maxBytes );

    /** The transfer and distribution of a match from its settings, with the defaults (attachment, all) for what is missing or unknown */
    static TMailTransfer GetTransfer( const MULTIPLAY::CMatchInfo& info );
    static TMailDistribution GetDistribution( const MULTIPLAY::CMatchInfo& info );
};

/*-------------------------------------------------------------------------*/

/**
 *  A signed statement of a participant about a match that has no record of its own: the reminder (nudge) and the
 *  leave (resign). It names the match, the signer and, for a reminder, the one who is reminded and the revision
 *  that is waited for. The signature has the match id as context, like the records of the generic layer.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailSignedStatement : public CORE::CIDataNodeSerializable
{
    public:

    static const char* KindNudge;               /**< "nudge" */
    static const char* KindResign;              /**< "resign" */

    CORE::CString matchId;
    CORE::CString statementKind;
    CORE::CString userId;                       /**< the signer */
    CORE::CString targetUserId;
    UInt32 revision;
    Int64 createdAt;
    CORE::CString signature;                    /**< hex */

    CMailSignedStatement( void );

    CMailSignedStatement( const CMailSignedStatement& src );

    virtual ~CMailSignedStatement() GUCEF_VIRTUAL_OVERRIDE;

    CMailSignedStatement& operator=( const CMailSignedStatement& src );

    /** Fills in the signer and signs */
    bool Sign( const MULTIPLAY::CPlayerIdentity& identity );

    /** Whether the signer is a participant of the match, with the pinned key, and the signature is valid for this kind of statement and this match */
    bool Verify( const MULTIPLAY::CMatchInfo& info, CORE::CString& reason ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    private:

    void GetSigningBytes( CORE::CDynamicBuffer& bytes ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H ? */
