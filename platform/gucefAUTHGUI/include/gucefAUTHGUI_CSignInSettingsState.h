/*
 *  gucefAUTHGUI: GUCEF module providing prebaked account and login user interface forms
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

#ifndef GUCEF_AUTHGUI_CSIGNINSETTINGSSTATE_H
#define GUCEF_AUTHGUI_CSIGNINSETTINGSSTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTHGUI_MACROS_H
#include "gucefAUTHGUI_macros.h"
#define GUCEF_AUTHGUI_MACROS_H
#endif /* GUCEF_AUTHGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a CSignInSettingsForm shows about the sign in of one service: the name, whether the person is signed in or a
 *  sign in runs, the saved credentials and the state of the keyring code that protects them. The application owns the
 *  facts and gives the form a copy with CSignInSettingsForm::SetState() whenever one changes; nothing in it is secret.
 */
class GUCEF_AUTHGUI_EXPORT_CPP CSignInSettingsState
{
    public:

    /** The keyring code of the saved credentials (gucefAUTH, CFileCredentialStore in the keyring code mode) */
    enum EKeyringState
    {
        KEYRING_UNAVAILABLE = 0,    /**< this build cannot keep saved credentials (no codec): the credential parts are disabled */
        KEYRING_NOT_SET_UP,         /**< no code yet: saving credentials first sets one up */
        KEYRING_LOCKED,             /**< the code has to be given before credentials can be saved or used */
        KEYRING_UNLOCKED
    };

    CORE::CString serviceName;                          /**< the name of the service as the person knows it */
    bool signedIn;                                      /**< there is a session */
    bool signInRunning;                                 /**< a sign in is under way: signing in and out wait */
    CORE::CString statusNote;                           /**< one more line after the state, such as "The session ended."; empty for none */
    bool hasSavedCredentials;                           /**< credentials are saved; only known while the keyring is unlocked */
    EKeyringState keyringState;
    CORE::CString::StringVector credentialSlots;        /**< the names of the credentials the service needs ("username", "password"): one secret field each */

    CSignInSettingsState( void );

    bool operator==( const CSignInSettingsState& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CSIGNINSETTINGSSTATE_H ? */
