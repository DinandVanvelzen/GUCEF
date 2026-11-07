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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTIONS_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTION_H
#include "matchprovidermail_CMailIntroduction.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTION_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTION_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILADDRESSBOOK_H
#include "matchprovidermail_CMailAddressBook.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILADDRESSBOOK_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILADDRESSBOOK_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a creator enters for one remote player */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroContactInput
{
    public:

    CORE::CString name;
    CORE::CString address;
    CORE::CString playerCode;           /**< the public key in hex when the creator has it already, empty otherwise */

    CMailIntroContactInput( void );
    CMailIntroContactInput( const CORE::CString& contactName, const CORE::CString& contactAddress, const CORE::CString& code = CORE::CString::Empty );
};

/** One remote player of a draft */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroContact
{
    public:

    CORE::CString name;                 /**< as the creator wrote it */
    CORE::CString address;              /**< lower case */
    CORE::CString nonce;                /**< 32 hex digits of the request that is out, empty when none */
    TMailIntroState state;
    TMailIntroSource source;
    CORE::CString publicKey;            /**< of an introduced contact */
    CORE::CString displayName;          /**< the name the contact gave in the reply, empty otherwise */
    CORE::CString codeKey;              /**< the player code the creator entered, empty for none */
    CORE::CString pendingKey;           /**< KEY_CHANGED: the key of the reply that waits for a decision */
    CORE::CString pendingName;
    Int64 requestedAt;                  /**< the first request */
    Int64 expiresAt;                    /**< of the request that is out */
    Int64 lastSentAt;
    UInt32 sendCount;

    CMailIntroContact( void );
};

/** A game that is waiting for the introductions of its players */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroDraft
{
    public:

    CORE::CString matchId;              /**< the id of the match to come */
    CORE::CString title;
    CORE::CString creatorName;
    CORE::CString creatorAddress;
    Int64 createdAt;
    GUCEF::map< CORE::CString, CORE::CString > settings;    /**< what the application needs to make the game when the draft is complete; opaque here */
    GUCEF::vector< CMailIntroContact > contacts;

    CMailIntroDraft( void );

    CMailIntroContact* FindContact( const CORE::CString& address );
    const CMailIntroContact* FindContact( const CORE::CString& address ) const;
};

/** How a draft stands */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroSummary
{
    public:

    UInt32 total;
    UInt32 notAsked;
    UInt32 waiting;
    UInt32 introduced;
    UInt32 declined;
    UInt32 expired;
    UInt32 keyChanged;
    UInt32 refused;
    bool allIntroduced;                 /**< every contact is introduced */
    bool startable;                     /**< at least one is introduced and none waits, is not asked or needs a decision about its key */

    CMailIntroSummary( void );
};

/** A request that came in and has not been answered, or that was answered */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailInboundIntro
{
    public:

    enum EStatus
    {
        STATUS_PENDING = 0 ,
        STATUS_ACCEPTED ,
        STATUS_DECLINED ,
        STATUS_CANCELLED                /**< the creator gave the game up before it began: kept until the player dismisses it or it runs out, so the list can say who cancelled */
    };

    CMailIntroRequest request;
    CORE::CString fromAddress;          /**< lower case */
    Int64 receivedAt;
    EStatus status;
    bool keyChanged;                    /**< the key of the creator differs from the address book */
    CORE::CString knownKey;             /**< the key the address book holds for the creator, for the display */
    Int64 lastReplyAt;

    CMailInboundIntro( void );
};

/** The limits and the clock-free settings of the introductions */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroLimits
{
    public:

    Int64 expiryMs;                     /**< how long a request is good for */
    Int64 resendIntervalMs;             /**< a request or a reply is sent again at most this often */
    UInt32 maxPerSender;                /**< pending requests per sender address */
    UInt32 maxPending;                  /**< pending requests in all */
    UInt32 maxDrafts;
    Int64 maxFutureSkewMs;              /**< a request dated further ahead than this is not believed */

    CMailIntroLimits( void );
};

/**
 *  The rules of the introduction handshake (design 18): the drafts of the games the player makes, the requests that other players
 *  sent, the answers, the expiry and the address book, kept in files under one directory. There is no mail and no clock here: the
 *  provider passes the mail it received and the time, and sends what these functions made.
 *
 *  Everything that comes in is hostile and is checked here: sizes, hex, the signature, the addresses against the From address of the mail,
 *  the nonce (compared in constant time), expiry, the keys against the address book. Nothing is written to a log with an address, a key
 *  or a nonce in it.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroductions
{
    public:

    static const UInt32 MaxContacts = CMailIntroRequest::MaxParties - 1;
    static const UInt32 MaxFileBytes = 4 * 1024 * 1024;
    static const char* const StateFileName;         /**< "introductions.state" */
    static const char* const BookFileName;          /**< "addressbook.state" */

    CMailIntroductions( void );
    ~CMailIntroductions();

    /** The identity signs what the player sends; a copy is kept. The directory is where the files are; empty keeps everything in memory */
    void Configure( const MULTIPLAY::CPlayerIdentity& identity, const CORE::CString& directory, const CMailIntroLimits& limits );

    /** Reads the files; what is not usable is ignored (the state starts empty) */
    bool Load( void );

    bool Save( void ) const;

    CMailAddressBook& GetBook( void );
    const CMailAddressBook& GetBook( void ) const;

    /* ---- the creator ---- */

    /**
     *  Starts a draft. Contacts with a player code are introduced at once, the others are NOT_ASKED (a key that the address book has for
     *  the address is what their answer is compared with, it does not replace the question). False with a reason (no addresses, no keys) when the input is not valid: the id, no title, 1 to MaxContacts contacts,
     *  addresses valid and different from each other and from the creator, a player code that is not a key or is the creator's or twice
     */
    bool BeginDraft( const CORE::CString& matchId, const CORE::CString& title, const CORE::CString& creatorName, const CORE::CString& creatorAddress,
                     const GUCEF::map< CORE::CString, CORE::CString >& settings, const GUCEF::vector< CMailIntroContactInput >& contacts, Int64 now, CORE::CString& reason );

    bool FindDraft( const CORE::CString& matchId, CMailIntroDraft& draft ) const;

    void GetDrafts( GUCEF::vector< CMailIntroDraft >& drafts ) const;

    bool GetSummary( const CORE::CString& matchId, CMailIntroSummary& summary ) const;

    bool RemoveDraft( const CORE::CString& matchId );

    /** Takes a contact out of a draft (the creator removes a player before the game is made). False when the draft or the contact is not there or it is the last contact */
    bool RemoveContact( const CORE::CString& matchId, const CORE::CString& address );

    /**
     *  Adds a player to a draft that began, checked like the players the draft began with (name, address, player code, no address or key twice, at
     *  most MaxContacts). A player with a player code is introduced at once, the others are asked by PrepareRequest() like the first ones
     */
    bool AddContact( const CORE::CString& matchId, const CMailIntroContactInput& input, CORE::CString& reason );

    /** Changes a setting the application keeps with the draft; the contacts that were told the earlier settings are asked to accept the table by hand when the game arrives */
    bool SetDraftSetting( const CORE::CString& matchId, const CORE::CString& key, const CORE::CString& value );

    /**
     *  Makes the signed request to send to a contact and marks the contact as waiting. A contact that is introduced is not asked unless 'force'.
     *  The same nonce is used while it is waiting and valid; a contact whose nonce is used up (declined, expired, refused, introduced with 'force')
     *  gets a new one. At most one request per resendIntervalMs per contact (RATE_LIMITED), the first one is not limited.
     */
    TMailIntroResult PrepareRequest( const CORE::CString& matchId, const CORE::CString& address, bool force, Int64 now, CMailIntroRequest& request );

    /** The reply of an invitee, from the mail with the From address 'fromAddress' */
    TMailIntroResult HandleReply( const CMailIntroReply& reply, const CORE::CString& fromAddress, Int64 now, CORE::CString* matchIdOut = GUCEF_NULL );

    /** The creator decides about a key that differs from the book: accepted it is used and remembered, refused the contact is REFUSED */
    TMailIntroResult ConfirmKeyChange( const CORE::CString& matchId, const CORE::CString& address, bool accept, Int64 now );

    /** Contacts whose request is past its expiry become EXPIRED; the ids of the drafts that changed are added to 'changed' */
    void ExpireDue( Int64 now, GUCEF::vector< CORE::CString >& changed );

    /** The introduced contacts of a draft in the order of the table: name, address and key */
    bool GetIntroduced( const CORE::CString& matchId, GUCEF::vector< CMailIntroContact >& contacts ) const;

    /* ---- the invitee ---- */

    /** A request that came by mail from 'fromAddress' to the player's own mailbox 'ownAddress'. 'matchKnown' says the provider has this match id already */
    TMailIntroResult HandleRequest( const CMailIntroRequest& request, const CORE::CString& fromAddress, const CORE::CString& ownAddress, bool matchKnown, Int64 now );

    void GetPending( GUCEF::vector< CMailInboundIntro >& pending ) const;

    bool FindInbound( const CORE::CString& matchId, CMailInboundIntro& intro ) const;

    /**
     *  Agrees: makes the signed reply. NEEDS_CONFIRMATION when the key of the creator differs from the book and 'confirmKeyChange' is false.
     *  The key of the creator is remembered. EXPIRED when the request is past its expiry.
     */
    TMailIntroResult AcceptRequest( const CORE::CString& matchId, bool confirmKeyChange, const CORE::CString& displayName, Int64 now, CMailIntroReply& reply );

    /** Declines: makes the decline once, a second call is a DUPLICATE */
    TMailIntroResult DeclineRequest( const CORE::CString& matchId, Int64 now, CMailIntroReply& reply );

    /** The reply of a request that was agreed to, made again (a lost reply); RATE_LIMITED inside the resend interval */
    TMailIntroResult RepeatReply( const CORE::CString& matchId, const CORE::CString& displayName, Int64 now, CMailIntroReply& reply );

    /**
     *  The creator cancels the game that asked to introduce the player: 'cancel' is a signed proposal of the kind CANCEL, checked against the key
     *  of the creator in the request. The request is no longer pending (CANCELLED, kept for display). OK, NO_REQUEST, BAD_SIGNATURE or DUPLICATE
     */
    TMailIntroResult CancelInbound( const CORE::CString& matchId, const MULTIPLAY::CMatchProposal& cancel );

    /** The invitation of the game arrived: the pending request has done its work and is dropped */
    void ForgetInbound( const CORE::CString& matchId );

    /** Whether every address of the table of a record is one the request announced (the creator's included) */
    bool IsTableAnnounced( const CORE::CString& matchId, const GUCEF::vector< CORE::CString >& addresses ) const;

    /** As above, and the settings of the record are exactly the ones the request announced (a request that announced none asks nothing of the record) */
    bool IsTableAnnounced( const CORE::CString& matchId, const GUCEF::vector< CORE::CString >& addresses, const GUCEF::map< CORE::CString, CORE::CString >& settings ) const;

    /**
     *  As above, with one setting (the number of seats, say) that the record may hold lower than announced: the creator took a player out of the draft
     *  after the request was sent, and the players who agreed need not agree again to a table that only got smaller. Every other setting is as announced.
     *  An empty key makes it the same as the overload above
     */
    bool IsTableAnnounced( const CORE::CString& matchId, const GUCEF::vector< CORE::CString >& addresses, const GUCEF::map< CORE::CString, CORE::CString >& settings, const CORE::CString& reducibleKey ) const;

    /** Pending requests past their expiry go; returns how many */
    UInt32 PurgeExpired( Int64 now );

    private:

    bool CheckContact( const CMailIntroDraft& draft, const CMailIntroContactInput& input, CORE::CString::StringSet& keys, CMailIntroContact& contact, CORE::CString& reason ) const;

    CMailIntroDraft* FindDraftPtr( const CORE::CString& matchId );
    const CMailIntroDraft* FindDraftPtr( const CORE::CString& matchId ) const;
    CMailInboundIntro* FindInboundPtr( const CORE::CString& matchId );
    const CMailInboundIntro* FindInboundPtr( const CORE::CString& matchId ) const;
    static CORE::CString MakeNonce( void );
    void ArmContact( const CMailIntroDraft& draft, CMailIntroContact& contact, Int64 now ) const;
    void MakeReply( const CMailInboundIntro& intro, const CORE::CString& displayName, Int64 now, CMailIntroReply& reply ) const;
    bool IsStatePersistent( void ) const;
    CORE::CString GetPath( const char* name ) const;
    void ChangedState( void );

    private:

    MULTIPLAY::CPlayerIdentity m_identity;
    CORE::CString m_directory;
    CMailIntroLimits m_limits;
    CMailAddressBook m_book;
    GUCEF::vector< CMailIntroDraft > m_drafts;
    GUCEF::vector< CMailInboundIntro > m_inbound;
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTIONS_H ? */
