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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The answer of the invitee: their key, name and address with the nonce of the request, signed with kind "intro-reply", or a decline
 *  that holds the nonce and nothing else (no key, no signature: declining hands nobody a key).
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIntroReply : public CORE::CIDataNodeSerializable
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

    CEMailIntroReply( void );
    CEMailIntroReply( const CEMailIntroReply& src );
    virtual ~CEMailIntroReply() GUCEF_VIRTUAL_OVERRIDE;
    CEMailIntroReply& operator=( const CEMailIntroReply& src );

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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H ? */
