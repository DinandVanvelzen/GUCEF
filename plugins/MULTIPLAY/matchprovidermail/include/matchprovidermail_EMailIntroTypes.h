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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailIntroStateToString( TMailIntroState state );

/*-------------------------------------------------------------------------*/

/** Where the key of an introduced contact comes from */
enum EMailIntroSource
{
    MAILINTROSOURCE_NONE  = 0 ,
    MAILINTROSOURCE_REPLY = 1 ,     /**< the signed reply of the contact */
    MAILINTROSOURCE_CODE  = 2       /**< a player code the creator entered */
};
typedef enum EMailIntroSource TMailIntroSource;

/*-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailIntroResultToString( TMailIntroResult result );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILINTROTYPES_H ? */
