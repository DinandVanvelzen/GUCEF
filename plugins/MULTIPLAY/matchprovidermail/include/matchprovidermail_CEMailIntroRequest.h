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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H
#include "matchprovidermail_CEMailIntroParty.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H ? */

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
 *  The signed invitation to introduce oneself: the creator says who they are (key, name, address), who is invited to which
 *  game with whom, and a secret nonce that only the mailbox of the invitee has. Signed with kind "intro-request" and the match id
 *  of the game to come as context.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIntroRequest : public CORE::CIDataNodeSerializable
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
    GUCEF::vector< CEMailIntroParty > parties;       /**< everybody at the table, the creator included, as the creator wrote it */
    GUCEF::map< CORE::CString, CORE::CString > settings;    /**< the settings of the game to come, as the record will hold them: what the invitee agrees to by introducing themselves */
    Int64 createdAt;
    Int64 expiresAt;
    CORE::CString signature;                        /**< hex */

    CEMailIntroRequest( void );
    CEMailIntroRequest( const CEMailIntroRequest& src );
    virtual ~CEMailIntroRequest() GUCEF_VIRTUAL_OVERRIDE;
    CEMailIntroRequest& operator=( const CEMailIntroRequest& src );

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H ? */
