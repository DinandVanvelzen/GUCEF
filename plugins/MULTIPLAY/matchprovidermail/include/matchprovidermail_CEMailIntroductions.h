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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODUCTIONS_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODUCTIONS_H

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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H
#include "matchprovidermail_EMailIntroTypes.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H ? */
#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROFINGERPRINT_H
#include "matchprovidermail_CEMailIntroFingerprint.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROFINGERPRINT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROFINGERPRINT_H ? */
#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H
#include "matchprovidermail_CEMailIntroParty.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H ? */
#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#include "matchprovidermail_CEMailIntroRequest.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H ? */
#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H
#include "matchprovidermail_CEMailIntroReply.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILADDRESSBOOK_H
#include "matchprovidermail_CEMailAddressBook.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILADDRESSBOOK_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILADDRESSBOOK_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACTINPUT_H
#include "matchprovidermail_CEMailIntroContactInput.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACTINPUT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACTINPUT_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#include "matchprovidermail_CEMailIntroContact.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODRAFT_H
#include "matchprovidermail_CEMailIntroDraft.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODRAFT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODRAFT_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROSUMMARY_H
#include "matchprovidermail_CEMailIntroSummary.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROSUMMARY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROSUMMARY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINBOUNDINTRO_H
#include "matchprovidermail_CEMailInboundIntro.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINBOUNDINTRO_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINBOUNDINTRO_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROLIMITS_H
#include "matchprovidermail_CEMailIntroLimits.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROLIMITS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROLIMITS_H ? */

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

/**
 *  The rules of the introduction handshake (design 18): the drafts of the games the player makes, the requests that other players
 *  sent, the answers, the expiry and the address book, kept in files under one directory. There is no mail and no clock here: the
 *  provider passes the mail it received and the time, and sends what these functions made.
 *
 *  Everything that comes in is hostile and is checked here: sizes, hex, the signature, the addresses against the From address of the mail,
 *  the nonce (compared in constant time), expiry, the keys against the address book. Nothing is written to a log with an address, a key
 *  or a nonce in it.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIntroductions
{
    public:

    static const UInt32 MaxContacts = CEMailIntroRequest::MaxParties - 1;
    static const UInt32 MaxFileBytes = 4 * 1024 * 1024;
    static const char* const StateFileName;         /**< "introductions.state" */
    static const char* const BookFileName;          /**< "addressbook.state" */

    CEMailIntroductions( void );
    ~CEMailIntroductions();

    /** The identity signs what the player sends; a copy is kept. The directory is where the files are; empty keeps everything in memory */
    void Configure( const MULTIPLAY::CPlayerIdentity& identity, const CORE::CString& directory, const CEMailIntroLimits& limits );

    /** Reads the files; what is not usable is ignored (the state starts empty) */
    bool Load( void );

    bool Save( void ) const;

    CEMailAddressBook& GetBook( void );
    const CEMailAddressBook& GetBook( void ) const;

    /* ---- the creator ---- */

    /**
     *  Starts a draft. Contacts with a player code are introduced at once, the others are NOT_ASKED (a key that the address book has for
     *  the address is what their answer is compared with, it does not replace the question). False with a reason (no addresses, no keys) when the input is not valid: the id, no title, 1 to MaxContacts contacts,
     *  addresses valid and different from each other and from the creator, a player code that is not a key or is the creator's or twice
     */
    bool BeginDraft( const CORE::CString& matchId, const CORE::CString& title, const CORE::CString& creatorName, const CORE::CString& creatorAddress,
                     const GUCEF::map< CORE::CString, CORE::CString >& settings, const GUCEF::vector< CEMailIntroContactInput >& contacts, Int64 now, CORE::CString& reason );

    bool FindDraft( const CORE::CString& matchId, CEMailIntroDraft& draft ) const;

    void GetDrafts( GUCEF::vector< CEMailIntroDraft >& drafts ) const;

    bool GetSummary( const CORE::CString& matchId, CEMailIntroSummary& summary ) const;

    bool RemoveDraft( const CORE::CString& matchId );

    /** Takes a contact out of a draft (the creator removes a player before the game is made). False when the draft or the contact is not there or it is the last contact */
    bool RemoveContact( const CORE::CString& matchId, const CORE::CString& address );

    /**
     *  Adds a player to a draft that began, checked like the players the draft began with (name, address, player code, no address or key twice, at
     *  most MaxContacts). A player with a player code is introduced at once, the others are asked by PrepareRequest() like the first ones
     */
    bool AddContact( const CORE::CString& matchId, const CEMailIntroContactInput& input, CORE::CString& reason );

    /** Changes a setting the application keeps with the draft; the contacts that were told the earlier settings are asked to accept the table by hand when the game arrives */
    bool SetDraftSetting( const CORE::CString& matchId, const CORE::CString& key, const CORE::CString& value );

    /**
     *  Makes the signed request to send to a contact and marks the contact as waiting. A contact that is introduced is not asked unless 'force'.
     *  The same nonce is used while it is waiting and valid; a contact whose nonce is used up (declined, expired, refused, introduced with 'force')
     *  gets a new one. At most one request per resendIntervalMs per contact (RATE_LIMITED), the first one is not limited.
     */
    TMailIntroResult PrepareRequest( const CORE::CString& matchId, const CORE::CString& address, bool force, Int64 now, CEMailIntroRequest& request );

    /** The reply of an invitee, from the mail with the From address 'fromAddress' */
    TMailIntroResult HandleReply( const CEMailIntroReply& reply, const CORE::CString& fromAddress, Int64 now, CORE::CString* matchIdOut = GUCEF_NULL );

    /** The creator decides about a key that differs from the book: accepted it is used and remembered, refused the contact is REFUSED */
    TMailIntroResult ConfirmKeyChange( const CORE::CString& matchId, const CORE::CString& address, bool accept, Int64 now );

    /** Contacts whose request is past its expiry become EXPIRED; the ids of the drafts that changed are added to 'changed' */
    void ExpireDue( Int64 now, GUCEF::vector< CORE::CString >& changed );

    /** The introduced contacts of a draft in the order of the table: name, address and key */
    bool GetIntroduced( const CORE::CString& matchId, GUCEF::vector< CEMailIntroContact >& contacts ) const;

    /* ---- the invitee ---- */

    /** A request that came by mail from 'fromAddress' to the player's own mailbox 'ownAddress'. 'matchKnown' says the provider has this match id already */
    TMailIntroResult HandleRequest( const CEMailIntroRequest& request, const CORE::CString& fromAddress, const CORE::CString& ownAddress, bool matchKnown, Int64 now );

    void GetPending( GUCEF::vector< CEMailInboundIntro >& pending ) const;

    bool FindInbound( const CORE::CString& matchId, CEMailInboundIntro& intro ) const;

    /**
     *  Agrees: makes the signed reply. NEEDS_CONFIRMATION when the key of the creator differs from the book and 'confirmKeyChange' is false.
     *  The key of the creator is remembered. EXPIRED when the request is past its expiry.
     */
    TMailIntroResult AcceptRequest( const CORE::CString& matchId, bool confirmKeyChange, const CORE::CString& displayName, Int64 now, CEMailIntroReply& reply );

    /** Declines: makes the decline once, a second call is a DUPLICATE */
    TMailIntroResult DeclineRequest( const CORE::CString& matchId, Int64 now, CEMailIntroReply& reply );

    /** The reply of a request that was agreed to, made again (a lost reply); RATE_LIMITED inside the resend interval */
    TMailIntroResult RepeatReply( const CORE::CString& matchId, const CORE::CString& displayName, Int64 now, CEMailIntroReply& reply );

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

    bool CheckContact( const CEMailIntroDraft& draft, const CEMailIntroContactInput& input, CORE::CString::StringSet& keys, CEMailIntroContact& contact, CORE::CString& reason ) const;

    CEMailIntroDraft* FindDraftPtr( const CORE::CString& matchId );
    const CEMailIntroDraft* FindDraftPtr( const CORE::CString& matchId ) const;
    CEMailInboundIntro* FindInboundPtr( const CORE::CString& matchId );
    const CEMailInboundIntro* FindInboundPtr( const CORE::CString& matchId ) const;
    static CORE::CString MakeNonce( void );
    void ArmContact( const CEMailIntroDraft& draft, CEMailIntroContact& contact, Int64 now ) const;
    void MakeReply( const CEMailInboundIntro& intro, const CORE::CString& displayName, Int64 now, CEMailIntroReply& reply ) const;
    bool IsStatePersistent( void ) const;
    CORE::CString GetPath( const char* name ) const;
    void ChangedState( void );

    private:

    MULTIPLAY::CPlayerIdentity m_identity;
    CORE::CString m_directory;
    CEMailIntroLimits m_limits;
    CEMailAddressBook m_book;
    GUCEF::vector< CEMailIntroDraft > m_drafts;
    GUCEF::vector< CEMailInboundIntro > m_inbound;
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODUCTIONS_H ? */
