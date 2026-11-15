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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefAUTH_KeyringTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
KeyringResultToString( TKeyringResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case KEYRING_OK: return "ok";
        case KEYRING_WRONG_CODE: return "wrong code";
        case KEYRING_THROTTLED: return "throttled";
        case KEYRING_BUSY: return "busy";
        case KEYRING_NOT_SET_UP: return "not set up";
        case KEYRING_ALREADY_SET_UP: return "already set up";
        case KEYRING_INVALID_CODE: return "invalid code";
        case KEYRING_TAMPERED: return "tampered";
        case KEYRING_NEWER_VERSION: return "newer version";
        case KEYRING_NOT_KEYRING_MODE: return "not keyring mode";
        case KEYRING_UNAVAILABLE: return "unavailable";
        default: return "failed";
    }
}

/*-------------------------------------------------------------------------*/

const char*
KeyringLockReasonToString( TKeyringLockReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case KEYRINGLOCK_REQUESTED: return "requested";
        case KEYRINGLOCK_IDLE: return "idle";
        case KEYRINGLOCK_APPLICATION_END: return "application end";
        case KEYRINGLOCK_CHANGED_ELSEWHERE: return "changed elsewhere";
        default: return "forgotten";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
