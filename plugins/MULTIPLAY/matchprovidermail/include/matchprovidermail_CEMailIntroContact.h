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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H
#include "matchprovidermail_EMailIntroTypes.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H ? */
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

/** One remote player of a draft */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIntroContact
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

    CEMailIntroContact( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H ? */
