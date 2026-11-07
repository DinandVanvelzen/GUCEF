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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILSIGNEDSTATEMENT_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILSIGNEDSTATEMENT_H

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
 *  A signed statement of a participant about a match that has no record of its own: the reminder (nudge) and the
 *  leave (resign). It names the match, the signer and, for a reminder, the one who is reminded and the revision
 *  that is waited for. The signature has the match id as context, like the records of the generic layer.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailSignedStatement : public CORE::CIDataNodeSerializable
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

    CEMailSignedStatement( void );

    CEMailSignedStatement( const CEMailSignedStatement& src );

    virtual ~CEMailSignedStatement() GUCEF_VIRTUAL_OVERRIDE;

    CEMailSignedStatement& operator=( const CEMailSignedStatement& src );

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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILSIGNEDSTATEMENT_H ? */
