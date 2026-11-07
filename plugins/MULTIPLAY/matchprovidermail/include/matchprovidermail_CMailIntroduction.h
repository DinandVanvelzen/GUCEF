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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTION_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTION_H

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

/** Where a contact of a draft stands */
enum EMailIntroState
{
    MAILINTRO_NOT_ASKED   = 0 ,     /**< no request was made yet */
    MAILINTRO_WAITING     = 1 ,     /**< the request is out, no answer */
    MAILINTRO_INTRODUCED  = 2 ,     /**< the key of the contact is known */
    MAILINTRO_DECLINED    = 3 ,
    MAILINTRO_EXPIRED     = 4 ,
    MAILINTRO_KEY_CHANGED = 5 ,     /**< a reply came with a key that differs from the key of the address book: it waits for the creator */
    MAILINTRO_REFUSED     = 6       /**< the creator refused the changed key */
};
typedef enum EMailIntroState TMailIntroState;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailIntroStateToString( TMailIntroState state );

/** Where the key of an introduced contact comes from */
enum EMailIntroSource
{
    MAILINTROSOURCE_NONE  = 0 ,
    MAILINTROSOURCE_REPLY = 1 ,     /**< the signed reply of the contact */
    MAILINTROSOURCE_CODE  = 2       /**< a player code the creator entered */
};
typedef enum EMailIntroSource TMailIntroSource;

/** What became of a request, a reply or an action; the text is the code of the notice (no content) */
enum EMailIntroResult
{
    MAILINTRORESULT_OK = 0 ,
    MAILINTRORESULT_DUPLICATE ,         /**< the same thing again: ignored without a notice */
    MAILINTRORESULT_CONFLICT ,          /**< another answer with a nonce that is used */
    MAILINTRORESULT_NO_DRAFT ,
    MAILINTRORESULT_NO_CONTACT ,
    MAILINTRORESULT_NOT_WAITING ,
    MAILINTRORESULT_EXPIRED ,
    MAILINTRORESULT_BAD_NONCE ,
    MAILINTRORESULT_BAD_ADDRESS ,       /**< the From address, the address in the statement or the address of the contact do not agree */
    MAILINTRORESULT_BAD_SIGNATURE ,
    MAILINTRORESULT_BAD_STATEMENT ,     /**< out of bounds or not well formed */
    MAILINTRORESULT_BAD_KEY ,           /**< the creator's own key, the key of another contact, or not the player code that was entered */
    MAILINTRORESULT_KEY_CHANGED ,       /**< valid, but the key differs from the address book: waiting for a decision */
    MAILINTRORESULT_DECLINED ,
    MAILINTRORESULT_BLOCKED ,
    MAILINTRORESULT_LIMIT ,
    MAILINTRORESULT_NOT_FOR_ME ,
    MAILINTRORESULT_KNOWN_MATCH ,
    MAILINTRORESULT_RATE_LIMITED ,
    MAILINTRORESULT_RESEND_REPLY ,      /**< (invitee) a request that was agreed to came again: the reply is sent again */
    MAILINTRORESULT_NO_REQUEST ,
    MAILINTRORESULT_NEEDS_CONFIRMATION  /**< (invitee) the key of the creator differs from the address book */
};
typedef enum EMailIntroResult TMailIntroResult;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailIntroResultToString( TMailIntroResult result );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The short form of a key that people can compare over another channel */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroFingerprint
{
    public:

    /** The first 80 bits of the SHA-256 of the key as five groups of four upper case hex digits ("3FA9-12C0-B7D4-8E51-09AA"); empty when the text is not a public key of 64 hex digits */
    static CORE::CString Of( const CORE::CString& publicKeyHex );
};

/*-------------------------------------------------------------------------*/

/** A player as the creator wrote the table: a name and a mail address */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroParty
{
    public:

    CORE::CString name;
    CORE::CString address;

    CMailIntroParty( void );
    CMailIntroParty( const CORE::CString& partyName, const CORE::CString& partyAddress );
};

/*-------------------------------------------------------------------------*/

/**
 *  The signed invitation to introduce oneself: the creator says who they are (key, name, address), who is invited to which
 *  game with whom, and a secret nonce that only the mailbox of the invitee has. Signed with kind "intro-request" and the match id
 *  of the game to come as context.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroRequest : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxParties = 6;
    static const UInt32 MaxNameBytes = 64;
    static const UInt32 MaxTitleBytes = 80;
    static const UInt32 MaxAddressBytes = 254;
    static const UInt32 NonceHexSize = 32;
    static const UInt32 MaxDataBytes = 32 * 1024;
    static const UInt32 MaxSettings = 32;           /**< as many as a match record holds */
    static const UInt32 MaxSettingKeyBytes = 64;
    static const UInt32 MaxSettingValueBytes = 256;

    static const char* SignedKind;                  /**< "intro-request" */

    CORE::CString matchId;
    CORE::CString nonce;                            /**< 32 hex digits */
    CORE::CString creatorUserId;
    CORE::CString creatorPublicKey;
    CORE::CString creatorName;
    CORE::CString creatorAddress;
    CORE::CString inviteeName;
    CORE::CString inviteeAddress;
    CORE::CString title;
    GUCEF::vector< CMailIntroParty > parties;       /**< everybody at the table, the creator included, as the creator wrote it */
    GUCEF::map< CORE::CString, CORE::CString > settings;    /**< the settings of the game to come, as the record will hold them: what the invitee agrees to by introducing themselves */
    Int64 createdAt;
    Int64 expiresAt;
    CORE::CString signature;                        /**< hex */

    CMailIntroRequest( void );
    CMailIntroRequest( const CMailIntroRequest& src );
    virtual ~CMailIntroRequest() GUCEF_VIRTUAL_OVERRIDE;
    CMailIntroRequest& operator=( const CMailIntroRequest& src );

    /** Fills in the key and the user id of the identity and signs; false without a key pair or when a field is not valid */
    bool Sign( const MULTIPLAY::CPlayerIdentity& identity );

    /** Whether every field is within its bounds and the signature is the one of the key for this match; no clock, no addresses of the mail */
    bool Verify( CORE::CString& reason ) const;

    /** Whether the bounds hold (without the signature) */
    bool IsWellFormed( CORE::CString& reason ) const;

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

/*-------------------------------------------------------------------------*/

/**
 *  The answer of the invitee: their key, name and address with the nonce of the request, signed with kind "intro-reply", or a decline
 *  that holds the nonce and nothing else (no key, no signature: declining hands nobody a key).
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailIntroReply : public CORE::CIDataNodeSerializable
{
    public:

    static const char* SignedKind;                  /**< "intro-reply" */

    CORE::CString matchId;
    CORE::CString nonce;
    bool accepted;
    CORE::CString userId;
    CORE::CString publicKey;
    CORE::CString displayName;
    CORE::CString inviteeAddress;
    CORE::CString creatorAddress;
    Int64 createdAt;
    CORE::CString signature;                        /**< hex, empty for a decline */

    CMailIntroReply( void );
    CMailIntroReply( const CMailIntroReply& src );
    virtual ~CMailIntroReply() GUCEF_VIRTUAL_OVERRIDE;
    CMailIntroReply& operator=( const CMailIntroReply& src );

    /** An acceptance: the key, user id and name of the identity are filled in and the statement is signed */
    bool Sign( const MULTIPLAY::CPlayerIdentity& identity );

    /** Whether the fields are within their bounds; a decline holds nothing but the nonce and the addresses */
    bool IsWellFormed( CORE::CString& reason ) const;

    /** Whether the reply is well formed and, for an acceptance, the user id is the one of the key and the signature is valid for this match */
    bool Verify( CORE::CString& reason ) const;

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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTION_H ? */
