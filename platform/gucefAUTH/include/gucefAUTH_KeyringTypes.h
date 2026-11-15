/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
 *
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

#ifndef GUCEF_AUTH_KEYRINGTYPES_H
#define GUCEF_AUTH_KEYRINGTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The outcome of an operation on a credential store that is sealed with a keyring code (CFileCredentialStore, CKeyringSession) */
enum EKeyringResult
{
    KEYRING_OK = 0,                 /**< done */
    KEYRING_WRONG_CODE,             /**< the code does not open the keyring: its check value does not match */
    KEYRING_THROTTLED,              /**< after wrong codes the next attempt has to wait (CKeyringSession::GetRetryWaitInMs()) */
    KEYRING_BUSY,                   /**< another operation of the session with a code runs */
    KEYRING_NOT_SET_UP,             /**< there is no keyring yet: set up a code first */
    KEYRING_ALREADY_SET_UP,         /**< a keyring exists: unlock it or change its code */
    KEYRING_INVALID_CODE,           /**< a code that is empty or longer than allowed */
    KEYRING_TAMPERED,               /**< the code is right, the sealed data fails authentication or is not a keyring file at all */
    KEYRING_NEWER_VERSION,          /**< the keyring file is of a newer format, it is left alone */
    KEYRING_NOT_KEYRING_MODE,       /**< the store is not sealed with a keyring code */
    KEYRING_UNAVAILABLE,            /**< the codecs it needs ("argon2id", "xchacha20-poly1305") are not there */
    KEYRING_FAILED                  /**< the files could not be read or written, the store stayed locked by another instance, no store */
};
typedef enum EKeyringResult TKeyringResult;

/** Why a keyring was locked */
enum EKeyringLockReason
{
    KEYRINGLOCK_REQUESTED = 0,      /**< the application or the person asked for it */
    KEYRINGLOCK_IDLE,               /**< it was not used for the idle time of the settings */
    KEYRINGLOCK_APPLICATION_END,    /**< the application ends, or the session was shut down */
    KEYRINGLOCK_CHANGED_ELSEWHERE,  /**< another instance changed the code, or the file was replaced or removed */
    KEYRINGLOCK_FORGOTTEN           /**< the keyring and its saved credentials were deleted */
};
typedef enum EKeyringLockReason TKeyringLockReason;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A stable lower case name of a result, for logs */
GUCEF_AUTH_EXPORT_CPP const char*
KeyringResultToString( TKeyringResult result );

/** A stable lower case name of a reason, for logs */
GUCEF_AUTH_EXPORT_CPP const char*
KeyringLockReasonToString( TKeyringLockReason reason );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_KEYRINGTYPES_H ? */
