/*
 *  AssetWorkbenchLib: the sign in of the workbench: the credential stores, the keyring code and the routing of the sign in of an online store
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHCREDENTIALS_H
#define ASSETWORKBENCHLIB_CWORKBENCHCREDENTIALS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CKEYRINGSESSION_H
#include "gucefAUTH_CKeyringSession.h"
#define GUCEF_AUTH_CKEYRINGSESSION_H
#endif /* GUCEF_AUTH_CKEYRINGSESSION_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H
#include "AssetWorkbenchLib_CAssetSourcesSettings.h"
#define ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H
#endif /* ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The credential stores of the workbench (design 12.1, 12.2): in one folder of the settings of the user, a key file store for the
 *  sessions of the online stores (the cookies of a web session: a session is used again after a restart without asking for
 *  anything) and a keyring code store for the passwords the owner chose to save (they need the keyring code, asked once per
 *  session of the workbench), and one CKeyringSession of the application on the second, with the idle lock of the settings.
 *
 *  Install() hands the two stores to ASSETSTORE::CAssetStoreGlobal::SetWebSignInStores(), which the web session store plugins
 *  take when they make their provider, so the workbench installs them before the plugins load; Shutdown() locks the keyring and
 *  takes the stores away again. Nothing here holds a code, a password or a cookie in a setting or a log: the stores seal what
 *  they keep and the keyring code is only ever in a field of the keyring form and in the derivation of the session.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchCredentials
{
    public:

    /** The folder "Auth" of the settings folder of the workbench, path variables not yet resolved */
    static CORE::CString GetDefaultFolder( void );

    CWorkbenchCredentials( void );

    ~CWorkbenchCredentials();

    /** Puts the stores in the folder (resolved, made when it is not there yet); false when the folder is empty */
    bool Open( const CORE::CString& folder );

    /** What the settings say: the folder (when it is not empty) and the idle lock of the keyring */
    void ApplySettings( const CAssetSourcesSettings& settings );

    /** Gives the stores to the asset store module for the plugins of the stores; an application does this before it loads them */
    void Install( void );

    /** Locks the keyring, ends what its session does and takes the stores away from the asset store module */
    void Shutdown( void );

    bool IsInstalled( void ) const;

    /** Whether this build can keep sessions and passwords: the codecs of the stores are loaded */
    bool IsKeyringAvailable( void ) const;

    CORE::CString GetFolder( void ) const;

    AUTH::CFileCredentialStore& GetSessionStore( void );
    AUTH::CFileCredentialStore& GetKeyringStore( void );
    AUTH::CKeyringSession& GetKeyringSession( void );

    private:

    CWorkbenchCredentials( const CWorkbenchCredentials& src );              /**< not implemented */
    CWorkbenchCredentials& operator=( const CWorkbenchCredentials& src );   /**< not implemented */

    private:

    AUTH::CFileCredentialStore m_sessionStore;
    AUTH::CFileCredentialStore m_keyringStore;
    AUTH::CKeyringSession m_session;
    bool m_installed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHCREDENTIALS_H ? */
