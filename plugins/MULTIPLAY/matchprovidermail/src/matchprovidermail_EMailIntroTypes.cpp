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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "matchprovidermail_EMailIntroTypes.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MailIntroStateToString( TMailIntroState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case MAILINTRO_NOT_ASKED:   return "NOT_ASKED";
        case MAILINTRO_WAITING:     return "WAITING";
        case MAILINTRO_INTRODUCED:  return "INTRODUCED";
        case MAILINTRO_DECLINED:    return "DECLINED";
        case MAILINTRO_EXPIRED:     return "EXPIRED";
        case MAILINTRO_KEY_CHANGED: return "KEY_CHANGED";
        case MAILINTRO_REFUSED:     return "REFUSED";
        default:                    return "NOT_ASKED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
MailIntroResultToString( TMailIntroResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case MAILINTRORESULT_OK:                    return "OK";
        case MAILINTRORESULT_DUPLICATE:             return "DUPLICATE";
        case MAILINTRORESULT_CONFLICT:              return "CONFLICT";
        case MAILINTRORESULT_NO_DRAFT:              return "NO_DRAFT";
        case MAILINTRORESULT_NO_CONTACT:            return "NO_CONTACT";
        case MAILINTRORESULT_NOT_WAITING:           return "NOT_WAITING";
        case MAILINTRORESULT_EXPIRED:               return "EXPIRED";
        case MAILINTRORESULT_BAD_NONCE:             return "BAD_NONCE";
        case MAILINTRORESULT_BAD_ADDRESS:           return "BAD_ADDRESS";
        case MAILINTRORESULT_BAD_SIGNATURE:         return "BAD_SIGNATURE";
        case MAILINTRORESULT_BAD_STATEMENT:         return "BAD_STATEMENT";
        case MAILINTRORESULT_BAD_KEY:               return "BAD_KEY";
        case MAILINTRORESULT_KEY_CHANGED:           return "KEY_CHANGED";
        case MAILINTRORESULT_DECLINED:              return "DECLINED";
        case MAILINTRORESULT_BLOCKED:               return "BLOCKED";
        case MAILINTRORESULT_LIMIT:                 return "LIMIT";
        case MAILINTRORESULT_NOT_FOR_ME:            return "NOT_FOR_ME";
        case MAILINTRORESULT_KNOWN_MATCH:           return "KNOWN_MATCH";
        case MAILINTRORESULT_RATE_LIMITED:          return "RATE_LIMITED";
        case MAILINTRORESULT_RESEND_REPLY:          return "RESEND_REPLY";
        case MAILINTRORESULT_NO_REQUEST:            return "NO_REQUEST";
        case MAILINTRORESULT_NEEDS_CONFIRMATION:    return "NEEDS_CONFIRMATION";
        default:                                    return "BAD_STATEMENT";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
