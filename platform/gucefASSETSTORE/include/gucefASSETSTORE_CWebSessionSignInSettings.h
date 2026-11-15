/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNINSETTINGS_H
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNINSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H
#include "gucefASSETSTORE_CWebLoginScript.h"
#define GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H
#endif /* GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H
#include "gucefASSETSTORE_CWebSignInBrowserSettings.h"
#define GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the web session sign in of one store (CWebSessionSignIn): the store, its login script, the browser
 *  and the time limits.
 *
 *  The browser profile is a folder used only for this store: browser.profileDirectory, else
 *  "$APPDATADIR$/<applicationName>/WebSignIn/<storeId>" (GetProfileDirectory()).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebSessionSignInSettings
{
    public:

    static const UInt32 DefaultSignInTimeoutInMs;      /**< 900000: fifteen minutes for the whole sign in */
    static const UInt32 DefaultHandOverTimeoutInMs;    /**< 600000: ten minutes for the owner to do a step of a person */
    static const UInt32 DefaultPollIntervalInMs;       /**< 250 */
    static const CORE::CString DefaultApplicationName; /**< "GUCEF" */

    CORE::CString storeId;                  /**< letters, digits, '_' and '-': also the name of the profile folder */
    CORE::CString displayName;              /**< empty: the name of the script */
    CWebLoginScript script;
    CWebSignInBrowserSettings browser;
    UInt32 signInTimeoutInMs;
    UInt32 handOverTimeoutInMs;
    UInt32 pollIntervalInMs;
    bool allowUnattendedRenewal;            /**< RenewSession() may sign in without a window with saved credentials */
    bool saveSession;                       /**< keep the session in the session store, so it is used again after a restart */
    CORE::CString applicationName;          /**< for the default profile folder */

    CWebSessionSignInSettings( void );

    CORE::CString GetDisplayName( void ) const;

    /** The profile folder of the browser, path variables not yet resolved */
    CORE::CString GetProfileDirectory( void ) const;

    /** The store id, the script and the time limits; the problems say what is wrong */
    bool Validate( CORE::CString::StringVector& problems ) const;

    /** Whether the id may be the id of a store here: letters, digits, '_' and '-', at most 64 */
    static bool IsValidStoreId( const CORE::CString& storeId );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNINSETTINGS_H ? */
